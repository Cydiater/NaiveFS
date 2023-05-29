#pragma once

#include "nfs/config.hpp"

class Inode {
  uint32_t size, access_time, modify_time, change_time;
  uint16_t uid, gid, link_cnt, mode;
  uint32_t directs[kInodeDirectCnt];
  uint32_t indirect1, indirect2;

public:
  Inode() {
    // todo
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    // todo
  }

  std::optional<uint32_t> find(const std::string &name) {
    // todo
  }
};