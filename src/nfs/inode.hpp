#pragma once

#include <cassert>
#include <cstring>
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

public:
  Inode(std::unique_ptr<DiskInode> disk_inode, SegmentBuilder *seg, Imap *imap)
      : disk_inode_(std::move(disk_inode)), seg_(seg), imap_(imap) {}

  std::unique_ptr<DiskInode> downgrade() {
    assert(disk_inode_ != nullptr);
    auto ret = std::unique_ptr<DiskInode>(nullptr);
    disk_inode_.swap(ret);
    return ret;
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    // todo
  }

  std::pair<std::vector<char *>, std::unique_ptr<DiskInode>>
  push(const std::string &name, const uint32_t inode_idx) {
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