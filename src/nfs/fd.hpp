#pragma once

#include <atomic>
#include <cassert>
#include <map>
#include <optional>

#include "nfs/config.hpp"
#include "nfs/utils.hpp"

class FDManager {
  std::map<uint32_t, uint32_t> fd2id;
  std::atomic<uint32_t> cnt_;

public:
  FDManager() : cnt_{2} {}

  uint32_t allocate(const uint32_t inode_idx) {
    auto fd = ++cnt_;
    fd2id[fd] = inode_idx;
    return fd;
  }

  uint32_t get(const uint32_t fd) {
    assert(fd >= 3);
    auto it = fd2id.find(fd);
    if (it == fd2id.end())
      throw NoFd();
    return it->second;
  }
};