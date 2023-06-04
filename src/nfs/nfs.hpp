#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>

#include "nfs/config.hpp"
#include "nfs/disk.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/fd.hpp"
#include "nfs/id.hpp"
#include "nfs/imap.hpp"
#include "nfs/inode.hpp"
#include "nfs/seg.hpp"
#include "nfs/utils.hpp"

class NaiveFS {
  std::unique_ptr<Disk> disk_;
  std::unique_ptr<SegmentsManager> seg_mgr_;
  std::unique_ptr<FDManager> fd_mgr_;
  std::unique_ptr<IDManager> id_mgr_;
  std::unique_ptr<Imap> imap_;

  enum class CR_DEST { START, END } last_cr_dest_;

  // We need to promote imap lock to this level
  // to prevent partial update. That is to say,
  // every atomic fs operation should acquire a shared
  // lock before any other update.
  std::shared_mutex lock_flushing_cr_;
  std::unique_ptr<std::thread> bg_thread_;

  void flush_cr() {
    debug("flushing checkpoint region");
    auto lock = std::unique_lock(lock_flushing_cr_);
    seg_mgr_->flush();
    const char *imap_buf = imap_->get_buf();
    const char *seg_buf = seg_mgr_->get_buf();
    char *newbuf = disk_->align_alloc(kCRSize);
    memcpy(newbuf, imap_buf, kCRImapSize);
    memcpy(newbuf + kCRImapSize, seg_buf, kCRSize - kCRImapSize);
    auto addr = last_cr_dest_ == CR_DEST::START ? disk_->end() - kCRSize : 0;
    last_cr_dest_ =
        last_cr_dest_ == CR_DEST::START ? CR_DEST::END : CR_DEST::START;
    disk_->write(newbuf, addr, kCRSize);
    disk_->sync();
    debug("flushed with version = " + std::to_string(imap_->version()) +
          " count = " + std::to_string(imap_->count()));
  }

  // running in a seperate thread
  void background() {
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(kCRFlushingSeconds));
      flush_cr();
    }
  }

public:
  NaiveFS()
      : disk_(std::make_unique<Disk>(kDiskPath, kDiskCapacityGB)),
        fd_mgr_(std::make_unique<FDManager>()),
        id_mgr_(std::make_unique<IDManager>()) {
    char *buf_start = new char[kCRImapSize];
    char *buf_end = new char[kCRImapSize];
    char *buf_seg_status = new char[kMaxSegments * 8];
    disk_->nread(buf_start, 0, kCRImapSize);
    disk_->nread(buf_end, disk_->end() - kCRSize, kCRImapSize);
    // non-aligned read
    auto imap1_ = std::make_unique<Imap>(buf_start);
    auto imap2_ = std::make_unique<Imap>(buf_end);
    if (imap1_->version() > imap2_->version()) {
      imap_.swap(imap1_);
      last_cr_dest_ = CR_DEST::START;
      disk_->read(buf_seg_status, kCRImapSize, kMaxSegments * 8);
      debug("use left CR");
    } else {
      imap_.swap(imap2_);
      last_cr_dest_ = CR_DEST::END;
      disk_->read(buf_seg_status, disk_->end() - kCRSize + kCRImapSize,
                  kMaxSegments * 8);
      debug("use right CR");
    }
    if (imap_->count() == 0) {
      auto root_inode = DiskInode::make_dir();
      auto addr = seg_mgr_->push(root_inode.get());
      imap_->update(id_mgr_->root_inode_idx, addr);
    }
    seg_mgr_ = std::make_unique<SegmentsManager>(disk_.get(), imap_.get(),
                                                 buf_seg_status);
    bg_thread_ = std::make_unique<std::thread>(&NaiveFS::background, this);
  }

  ~NaiveFS() { flush_cr(); }

  uint32_t open(const char *path, const int flags) {
    auto lock = std::shared_lock(lock_flushing_cr_);
    auto path_components = parse_path_components(path);
    assert(path_components.size() >= 1);
    auto name = path_components.back();
    path_components.pop_back();
    auto parent_path = join_path_components(path_components);
    auto parent_inode_idx = get_inode_idx(parent_path.c_str());
    auto parent_inode = get_inode(parent_inode_idx);
    auto maybe_this_inode_idx = parent_inode->find_entry(name);
    if (maybe_this_inode_idx.has_value()) {
      auto this_inode_idx = maybe_this_inode_idx.value();
      auto fd = fd_mgr_->allocate(this_inode_idx);
      return fd;
    }
    auto this_disk_inode = DiskInode::make_file();
    auto this_dinode_addr = seg_mgr_->push(this_disk_inode.get());
    auto this_inode_idx = id_mgr_->allocate();
    imap_->update(this_inode_idx, this_dinode_addr);
    debug("open this_inode_idx = " + std::to_string(this_inode_idx));
    auto nv_parent_disk_inode = parent_inode->push(name, this_inode_idx);
    auto nv_parent_dinode_addr = seg_mgr_->push(nv_parent_disk_inode.get());
    imap_->update(parent_inode_idx, nv_parent_dinode_addr);
    auto fd = fd_mgr_->allocate(this_inode_idx);
    return fd;
  }

  std::vector<std::string> readdir(const char *path) {
    auto lock = std::shared_lock(lock_flushing_cr_);
    auto inode_idx = get_inode_idx(path);
    auto inode = get_inode(inode_idx);
    auto names = inode->list_entries();
    return names;
  }

  void unlink(const char *path) {
    // todo: support real unlink after link implemented
    auto lock = std::shared_lock(lock_flushing_cr_);
    auto path_components = parse_path_components(path);
    assert(path_components.size() >= 1);
    auto name = path_components.back();
    path_components.pop_back();
    auto parent_path = join_path_components(path_components);
    auto parent_inode_idx = get_inode_idx(parent_path.c_str());
    debug("unlink parent_inode_idx = " + std::to_string(parent_inode_idx));
    auto parent_inode = get_inode(parent_inode_idx);
    auto nv_parent_disk_inode = parent_inode->erase_entry(name);
    if (nv_parent_disk_inode == nullptr) {
      throw NoEntry();
    }
    auto nv_parent_dinode_addr = seg_mgr_->push(nv_parent_disk_inode.get());
    imap_->update(parent_inode_idx, nv_parent_dinode_addr);
  }

  uint32_t read(const uint32_t fd, char *buf, uint32_t offset, uint32_t size) {
    auto lock = std::shared_lock(lock_flushing_cr_);
    debug("read " + std::to_string(size));
    auto inode_idx = fd_mgr_->get(fd);
    auto inode = get_inode(inode_idx);
    return inode->read(buf, offset, size);
  }

  void write(const uint32_t fd, char *buf, uint32_t offset, uint32_t size) {
    auto lock = std::shared_lock(lock_flushing_cr_);
    debug("FILE write size " + std::to_string(size) + " offset " +
          std::to_string(offset));
    auto inode_idx = fd_mgr_->get(fd);
    auto inode = get_inode(inode_idx);
    auto disk_inode = inode->write(buf, offset, size);
    auto new_addr = seg_mgr_->push(disk_inode.get());
    imap_->update(inode_idx, new_addr);
  }

  void modify(std::unique_ptr<DiskInode> disk_inode, const uint32_t inode_idx) {
    // todo
  }

  uint32_t get_inode_idx(const uint32_t fd) { return fd_mgr_->get(fd); }

  uint32_t get_inode_idx(const char *path) {
    auto inode_idx = id_mgr_->root_inode_idx;
    auto inode = get_inode(inode_idx);
    auto path_components = parse_path_components(path);
    for (const auto &com : path_components) {
      auto found = inode->find_entry(com);
      if (!found.has_value()) {
        throw NoEntry();
      }
      inode_idx = found.value();
      debug("get_inode_idx found " + com + " -> " + std::to_string(inode_idx));
      inode = get_inode(inode_idx);
      assert(inode != nullptr);
    }
    debug("get inode index " + std::string(path) + " -> " +
          std::to_string(inode_idx));
    return inode_idx;
  }

  std::unique_ptr<DiskInode> get_diskinode(const uint32_t inode_idx) {
    auto inode_addr = imap_->get(inode_idx);
    auto disk_inode = std::make_unique<DiskInode>();
    seg_mgr_->read(reinterpret_cast<char *>(disk_inode.get()), inode_addr,
                   sizeof(DiskInode));
    assert(disk_inode->link_cnt != 0);
    return disk_inode;
  }

private:
  std::unique_ptr<Inode> get_inode(const uint32_t inode_idx) {
    auto disk_inode = get_diskinode(inode_idx);
    if (disk_inode == nullptr)
      return nullptr;
    auto inode = std::make_unique<Inode>(std::move(disk_inode), seg_mgr_.get());
    return inode;
  }
};