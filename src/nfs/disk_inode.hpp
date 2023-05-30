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

  static std::unique_ptr<DiskInode> make_dir() {
    auto root = std::make_unique<DiskInode>();
    auto instant = time(nullptr);
    root->access_time = instant;
    root->modify_time = instant;
    root->change_time = instant;
    root->uid = getuid();
    root->gid = getgid();
    root->link_cnt = 1;
    root->mode = S_IFDIR;
    std::memset(root->directs, 0, sizeof(root->directs));
    root->indirect1 = root->indirect2 = 0;
    return root;
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
