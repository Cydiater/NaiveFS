#pragma once

#include "nfs/config.hpp"
#include "nfs/utils.hpp"

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class NaiveFS;

class Imap {
  uint32_t *map_;
  uint32_t active_count;
  uint32_t *version_;
  static const uint32_t INVALID_VALUE = 0;
  // todo: ensure thread safety
  // flushing imap to cr should ensure mutex against update

public:
  Imap(char *from)
      : map_(reinterpret_cast<uint32_t *>(from + 4)),
        version_(reinterpret_cast<uint32_t *>(from)) {
    active_count = 0;
    for (uint32_t i = 0; i < kMaxInode; i++) {
      active_count += (map_[i] != INVALID_VALUE);
    }
    debug(std::string("IMAP init with active_count = ") +
          std::to_string(active_count) +
          " version = " + std::to_string(version()));
  }

  ~Imap() { delete[](map_ - 1); }

  const char *get_buf() {
    *version_ = version() + 1;
    return reinterpret_cast<const char *>(version_);
  }

  uint32_t count() const { return active_count; }
  uint32_t version() const { return *version_; }

  uint32_t get(const uint32_t inode_idx) {
    assert(inode_idx < kMaxInode);
    auto entry = map_[inode_idx];
    if (entry == INVALID_VALUE) {
      debug(std::string("IMAP get inode_idx ") + std::to_string(inode_idx) +
            std::string(" not found"));
      throw NoImapEntry();
    }
    debug(std::string("IMAP get inode_idx ") + std::to_string(inode_idx) +
          std::string(" -> ") + std::to_string(entry));
    return entry;
  }

  void update(const uint32_t inode_idx, const uint32_t inode_addr) {
    debug(std::string("IMAP set inode_idx ") + std::to_string(inode_idx) +
          std::string(" -> ") + std::to_string(inode_addr));
    assert(inode_idx < kMaxInode);
    assert(inode_addr != INVALID_VALUE);
    if (map_[inode_idx] == INVALID_VALUE)
      active_count += 1;
    map_[inode_idx] = inode_addr;
  }
};