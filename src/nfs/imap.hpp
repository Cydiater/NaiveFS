#pragma once

#include "nfs/config.hpp"

#include <vector>

class Imap {
  uint32_t *map_;
  uint32_t active_count;
  static const uint32_t INVALID_VALUE = 0;
  // todo: ensure thread safety
  // flushing imap to cr should ensure mutex against update

public:
  Imap(const char *from) {
    active_count = 0;
    for (int i = 0; i < kMaxInode; i++) {
      active_count += (map_[i] != 0);
    }
  }

  uint32_t count() const { return active_count; }

  uint32_t get(const uint32_t inode_idx) {
    // todo
    return 0;
  }

  uint32_t update(const uint32_t inode_idx, const uint32_t inode_addr) {
    // todo
    return 0;
  }
};