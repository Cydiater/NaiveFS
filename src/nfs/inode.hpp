#pragma once

#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/imap.hpp"
#include "nfs/seg.hpp"
#include "nfs/utils.hpp"

class Inode {
  std::unique_ptr<DiskInode> disk_inode_;
  SegmentBuilder *seg_;
  bool dirty_;

  void for_each_block(
      uint32_t offset, const uint32_t size,
      std::function<uint32_t(const uint32_t, const uint32_t, const uint32_t)>
          callback) {
    assert(dirty_ == false);
    auto end = offset + size;
    uint32_t *indirect1 = nullptr;
    uint32_t indirect1_addr = 0;
    uint32_t *indirect2 = nullptr;
    uint32_t indirect2_addr = 0;
    std::ignore = indirect2_addr;
    if (end > disk_inode_->size) {
      disk_inode_->size = end;
      dirty_ = true;
    }
    while (offset < end) {
      const auto [i0, i1, i2] = DiskInode::translate(offset);
      debug("offset = " + std::to_string(offset) +
            " i0 = " + std::to_string(i0) + " i1 = " + std::to_string(i1) +
            " i2 = " + std::to_string(i2));
      const auto next_offset = (offset / kBlockSize + 1) * kBlockSize;
      const auto this_size = std::min(next_offset - offset, end - offset);
      if (i0 < kInodeDirectCnt) {
        auto this_addr = disk_inode_->directs[i0];
        auto new_addr = callback(this_addr, offset % kBlockSize, this_size);
        if (new_addr != this_addr) {
          dirty_ = true;
          debug("set disk_inode directs " + std::to_string(i0) + " -> " +
                std::to_string(new_addr));
          disk_inode_->directs[i0] = new_addr;
        }
      } else if (i0 < kInodeDirectCnt + 1) {
        if (indirect1_addr != disk_inode_->indirect1) {
          if (indirect1 == nullptr) {
            indirect1 = new uint32_t[kBlockSize / 4];
          }
          indirect1_addr = disk_inode_->indirect1;
          seg_->read(reinterpret_cast<char *>(indirect1), indirect1_addr,
                     kBlockSize);
        }
        auto this_addr = indirect1[i1];
        auto new_addr = callback(this_addr, next_offset, this_size);
        if (new_addr != this_addr) {
          dirty_ = true;
          indirect1[i1] = new_addr;
        }
      } else {
        // todo
        assert(false);
      }
      offset = next_offset;
    }
    if (indirect1 != nullptr) {
      if (dirty_) {
        auto addr = seg_->push(reinterpret_cast<const char *>(indirect1));
        disk_inode_->indirect1 = addr;
      }
      delete[] indirect1;
    }
    if (indirect2 != nullptr)
      delete[] indirect2;
  }

public:
  Inode(std::unique_ptr<DiskInode> disk_inode, SegmentBuilder *seg)
      : disk_inode_(std::move(disk_inode)), seg_(seg), dirty_(false) {}

  std::unique_ptr<DiskInode> downgrade() {
    assert(disk_inode_ != nullptr);
    auto ret = std::unique_ptr<DiskInode>(nullptr);
    disk_inode_.swap(ret);
    return ret;
  }

  std::unique_ptr<DiskInode> write(char *buf, uint32_t offset, uint32_t size) {
    assert(offset <= disk_inode_->size);
    for_each_block(offset, size,
                   [&buf, this](const uint32_t addr, const uint32_t this_offset,
                                const uint32_t this_size) {
                     if (this_size == kBlockSize) {
                       assert(this_offset == 0);
                       auto new_addr = seg_->push(buf);
                       return new_addr;
                     }
                     auto this_buf = new char[kBlockSize];
                     seg_->read(this_buf, addr, kBlockSize);
                     std::memcpy(this_buf, buf + this_offset, this_size);
                     auto new_addr = seg_->push(this_buf);
                     delete[] this_buf;
                     buf += this_size;
                     return new_addr;
                   });
    return downgrade();
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    assert(offset + size <= disk_inode_->size);
    for_each_block(offset, size,
                   [&buf, this](const uint32_t addr, const uint32_t this_offset,
                                const uint32_t this_size) {
                     debug(
                         "read for each block addr = " + std::to_string(addr) +
                         " this_offset = " + std::to_string(this_offset) +
                         " this_size = " + std::to_string(this_size));
                     seg_->read(buf, addr + this_offset, this_size);
                     buf += this_size;
                     return addr;
                   });
  }

  std::unique_ptr<DiskInode> push(const std::string &name,
                                  const uint32_t inode_idx) {
    debug("inode dir push entry " + name + " -> " + std::to_string(inode_idx));
    auto [buf, len] = make_one_dir_entry(name, inode_idx);
    return write(buf, disk_inode_->size, len);
  }

  std::optional<uint32_t> find(const std::string &name) {
    // assume that directory should not be very large here
    // fix: support large directory
    assert(disk_inode_->size <= kBlockSize * 3);
    if (disk_inode_->size == 0)
      return std::nullopt;
    debug("disk_inode_->size = " + std::to_string(disk_inode_->size));
    auto buf = new char[disk_inode_->size];
    read(buf, 0, disk_inode_->size);
    uint32_t offset = 0;
    while (offset < disk_inode_->size) {
      const auto [this_name, this_inode_idx] =
          parse_one_dir_entry(buf, offset, disk_inode_->size);
      debug("inode dir find name =  " + name + " this_name " + this_name +
            " -> " + std::to_string(this_inode_idx));
      if (this_name == name) {
        delete[] buf;
        return this_inode_idx;
      }
    }
    delete[] buf;
    return std::nullopt;
  }
};