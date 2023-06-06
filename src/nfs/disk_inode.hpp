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

  static constexpr uint32_t INVALID_ADDR = 0;
  static constexpr uint32_t TEMPORARY_ADDR =
      std::numeric_limits<uint32_t>::max();
  static constexpr uint32_t INVALID_INDIRECT_IDX = 4095;

  static std::unique_ptr<DiskInode> make() {
    auto disk_inode = std::make_unique<DiskInode>();
    auto instant = time(nullptr);
    disk_inode->access_time = instant;
    disk_inode->modify_time = instant;
    disk_inode->change_time = instant;
    disk_inode->uid = getuid();
    disk_inode->gid = getgid();
    disk_inode->link_cnt = 1;
    for (uint32_t i = 0; i < kInodeDirectCnt; i++)
      disk_inode->directs[i] = INVALID_ADDR;
    disk_inode->indirect1 = disk_inode->indirect2 = INVALID_ADDR;
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
            (offset - base2) / kBlockSize / (kBlockSize / 4),
            ((offset - base2) / kBlockSize) % (kBlockSize / 4)};
  }

  static std::string to_string(const uint32_t code) {
    const auto [i0, i1, i2] = decode(code);
    if (code & (1 << 29))
      return "[" + std::to_string(i0) + ", " + std::to_string(i1) + ", " +
             std::to_string(i2) + "]";
    if (code & (1 << 30))
      return "[" + std::to_string(i0) + ", " + std::to_string(i1) + "]";
    return "[" + std::to_string(i0) + "]";
  }

  static std::tuple<uint32_t, uint32_t, uint32_t> decode(const uint32_t code) {
    auto this_i0 = code & ((1 << 5) - 1);
    auto this_i1 = (code >> 5) & ((1 << 10) - 1);
    auto this_i2 = (code >> 15) & ((1 << 10) - 1);
    return {this_i0, this_i1, this_i2};
  }

  static uint32_t encode(const uint32_t i0) {
    auto this_i0 = i0 & ((1 << 5) - 1);
    assert(this_i0 == i0);
    return this_i0 | (1 << 31);
  }

  static uint32_t encode(const uint32_t i0, const uint32_t i1) {
    auto this_i0 = encode(i0);
    auto this_i1 = i1 & ((1 << 10) - 1);
    assert(this_i1 == i1);
    return this_i0 | (this_i1 << 5) | (1 << 30);
  }

  static uint32_t encode(const uint32_t i0, const uint32_t i1,
                         const uint32_t i2) {
    auto this_i01 = encode(i0, i1);
    auto this_i2 = i2 & ((1 << 10) - 1);
    assert(this_i2 == i2);
    return this_i01 | (this_i2 << 15) | (1 << 29);
  }
};
