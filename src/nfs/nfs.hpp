#pragma once

#include <cassert>
#include <memory>
#include <optional>

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
  std::unique_ptr<SegmentBuilder> seg_builder_;
  std::unique_ptr<FDManager> fd_mgr_;
  std::unique_ptr<IDManager> id_mgr_;
  std::unique_ptr<Imap> imap_;

public:
  NaiveFS()
      : disk_(std::make_unique<Disk>(kDiskPath, kDiskCapacityGB)),
        seg_builder_(std::make_unique<SegmentBuilder>(disk_.get())) {
    char *buf = new char[kCRSize];
    disk_->read(buf, 0, kCRSize);
    imap_ = std::make_unique<Imap>(buf);
    if (imap_->count() == 0) {
      auto root_inode = DiskInode::make_dir();
      auto addr = seg_builder_->push(root_inode.get());
      imap_->update(id_mgr_->root_inode_idx, addr);
    }
  }

  std::optional<uint32_t> open(const char *path, const int flags) {
    // todo
  }

  void readdir() {
    // todo
  }

  void read(const uint32_t fd, char *buf, uint32_t offset, uint32_t size) {
    // todo
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
      auto found = inode->find(com);
      if (!found.has_value()) {
        throw NoEntry();
      }
      inode_idx = found.value();
      inode = get_inode(inode_idx);
      assert(inode != nullptr);
    }
    return inode_idx;
  }

  std::unique_ptr<DiskInode> get_diskinode(const uint32_t inode_idx) {
    auto found = imap_->get(inode_idx);
    if (found == std::nullopt)
      return nullptr;
    auto inode_addr = found.value();
    auto disk_inode = std::make_unique<DiskInode>();
    seg_builder_->read(reinterpret_cast<char *>(disk_inode.get()), inode_addr,
                       sizeof(Inode));
    return disk_inode;
  }

private:
  std::unique_ptr<Inode> get_inode(const uint32_t inode_idx) {
    auto disk_inode = get_diskinode(inode_idx);
    if (disk_inode == nullptr)
      return nullptr;
    auto inode = std::make_unique<Inode>(std::move(disk_inode),
                                         seg_builder_.get(), imap_.get());
    return inode;
  }
};