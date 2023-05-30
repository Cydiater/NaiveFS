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
  Imap *imap_;

  void for_each_block(
      uint32_t offset, const uint32_t size,
      std::function<void(const uint32_t, const uint32_t, const uint32_t)>
          callback) {
    auto end = offset + size;
    uint32_t *indirect1 = nullptr;
    uint32_t indirect1_addr = 0;
    uint32_t *indirect2 = nullptr;
    uint32_t indirect2_addr = 0;
    std::ignore = indirect2_addr;
    while (offset < end) {
      const auto [i0, i1, i2] = disk_inode_->translate(offset);
      const auto next_offset = (offset / kBlockSize + 1) * kBlockSize;
      const auto this_size = std::min(next_offset - offset, kBlockSize);
      if (i0 < kInodeDirectCnt) {
        callback(disk_inode_->directs[i0], offset % kBlockSize, this_size);
        offset = next_offset;
      } else if (i0 < kInodeDirectCnt + 1) {
        if (indirect1_addr != disk_inode_->indirect1) {
          if (indirect1 == nullptr) {
            indirect1 = new uint32_t[kBlockSize / 4];
          }
          indirect1_addr = disk_inode_->indirect1;
          seg_->read(reinterpret_cast<char *>(indirect1), indirect1_addr,
                     kBlockSize);
        }
        callback(indirect1[i1], next_offset, this_size);
      } else {
        // todo
        assert(false);
      }
    }
    if (indirect1 != nullptr)
      delete[] indirect1;
    if (indirect2 != nullptr)
      delete[] indirect2;
  }

public:
  Inode(std::unique_ptr<DiskInode> disk_inode, SegmentBuilder *seg, Imap *imap)
      : disk_inode_(std::move(disk_inode)), seg_(seg), imap_(imap) {}

  std::unique_ptr<DiskInode> downgrade() {
    assert(disk_inode_ != nullptr);
    auto ret = std::unique_ptr<DiskInode>(nullptr);
    disk_inode_.swap(ret);
    return ret;
  }

  void write(char *buf, uint32_t offset, uint32_t size) {
    assert(offset <= disk_inode_->size);
    // todo
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    assert(offset + size < disk_inode_->size);
    for_each_block(offset, size,
                   [&buf, this](const uint32_t addr, const uint32_t this_offset,
                                const uint32_t this_size) {
                     seg_->read(buf, addr + this_offset, this_size);
                     buf += this_size;
                   });
  }

  std::unique_ptr<DiskInode> push(const std::string &name,
                                  const uint32_t inode_idx) {
    // todo
  }

  std::optional<uint32_t> find(const std::string &name) {
    // assume that directory should not be very large here
    // fix: support large directory
    assert(disk_inode_->size <= kBlockSize * 3);
    if (disk_inode_->size == 0)
      return std::nullopt;
    auto buf = new char[disk_inode_->size];
    read(buf, 0, disk_inode_->size);
    uint32_t offset = 0;
    while (offset < disk_inode_->size) {
      const auto [this_name, this_inode_idx] =
          parse_one_dir_entry(buf, offset, disk_inode_->size);
      if (this_name == name) {
        return this_inode_idx;
      }
    }
    return std::nullopt;
  }
};