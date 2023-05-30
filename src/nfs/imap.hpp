#pragma once

#include "nfs/config.hpp"
#include "nfs/utils.hpp"

#include <cassert>
#include <optional>
#include <vector>

class Imap {
  uint32_t *map_;
  uint32_t active_count;
  static const uint32_t INVALID_VALUE = 0;
  // todo: ensure thread safety
  // flushing imap to cr should ensure mutex against update

public:
  Imap(char *from) : map_(reinterpret_cast<uint32_t *>(from)) {
    active_count = 0;
    for (uint32_t i = 0; i < kMaxInode; i++) {
      active_count += (map_[i] != 0);
    }
  }

  uint32_t count() const { return active_count; }

  uint32_t get(const uint32_t inode_idx) {
    assert(inode_idx < kMaxInode);
    auto entry = map_[inode_idx];
    if (entry == INVALID_VALUE)
      throw NoImapEntry();
    return entry;
  }

  void update(const uint32_t inode_idx, const uint32_t inode_addr) {
    assert(inode_idx < kMaxInode);
    assert(inode_addr != INVALID_VALUE);
    map_[inode_idx] = inode_addr;
  }
};