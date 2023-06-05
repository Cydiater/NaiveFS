#pragma once

#include <cassert>
#include <cstring>
#include <limits>
#include <memory>

#include <sys/stat.h>
#include <unistd.h>

#include "nfs/config.hpp"

struct DiskInode {
  uint32_t size, access_time, modify_time, change_time;
  uint16_t uid, gid, link_cnt, mode;
  uint32_t directs[kInodeDirectCnt];
  uint32_t indirect1, indirect2;

  static std::unique_ptr<DiskInode> make() {
    auto disk_inode = std::make_unique<DiskInode>();
    auto instant = time(nullptr);
    disk_inode->access_time = instant;
    disk_inode->modify_time = instant;
    disk_inode->change_time = instant;
    disk_inode->uid = getuid();
    disk_inode->gid = getgid();
    disk_inode->link_cnt = 1;
    std::memset(disk_inode->directs, 0, sizeof(disk_inode->directs));
    disk_inode->indirect1 = disk_inode->indirect2 = 0;
    return disk_inode;
  }

  static std::unique_ptr<DiskInode> make_file() {
    auto disk_inode = make();
    disk_inode->mode = S_IFREG;
    return disk_inode;
  }

  static std::unique_ptr<DiskInode> make_dir() {
    auto disk_inode = make();
    disk_inode->mode = S_IFDIR;
    return disk_inode;
  }

  uint32_t st_blocks() {
    // todo
    return 0;
  }

  static std::tuple<uint32_t, uint32_t, uint32_t>
  translate(const uint32_t offset) {
    constexpr uint32_t base1 = kBlockSize * kInodeDirectCnt;
    constexpr uint32_t base2 = base1 + kBlockSize * (kBlockSize / 4);
    if (offset < base1)
      return {offset / kBlockSize, 0, 0};
    if (offset < base2)
      return {kInodeDirectCnt, (offset - base1) / kBlockSize, 0};
    return {kInodeDirectCnt + 1,
            ((offset - base2) / kBlockSize) % (kBlockSize / 4),
            (offset - base2) / kBlockSize / (kBlockSize / 4)};
  }

  static uint32_t encode(const uint32_t i0) {
    auto this_i0 = i0 & ((1 << 5) - 1);
    assert(this_i0 == i0);
    return this_i0;
  }

  static uint32_t encode(const uint32_t i0, const uint32_t i1) {
    // todo
  }

  static uint32_t encode(const uint32_t i0, const uint32_t i1,
                         const uint32_t i2) {
    // todo
  }
};
