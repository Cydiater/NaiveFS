#pragma once

#include "nfs/config.hpp"

#include <vector>

class Imap {
  std::vector<uint32_t> map_;
  // todo: ensure thread safety

public:
  static Imap *instance() { return nullptr; }

  uint32_t get(const uint32_t inode_idx) {
    // todo
  }

  uint32_t update(const uint32_t inode_idx, const uint32_t inode_addr) {
    // todo
  }
};