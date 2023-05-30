#pragma once

#include <cstring>
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
    disk_inode->mode = S_IFDIR;
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

  std::pair<uint32_t, uint32_t> translate(const uint32_t offset) {
    // todo
    return {};
  }
};
