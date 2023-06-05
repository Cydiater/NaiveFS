#pragma once

#include <atomic>

#include "nfs/config.hpp"

class IDManager {
  std::atomic<uint32_t> cur_idx;

public:
  static constexpr uint32_t root_inode_idx = 0;

  IDManager() { cur_idx = root_inode_idx; }

  int allocate() { return ++cur_idx; }
};