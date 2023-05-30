#pragma once

#include <map>

#include "nfs/config.hpp"

class FDManager {
  std::map<uint32_t, uint32_t> fd2id;

public:
  uint32_t allocate(const uint32_t inode_idx) {
    // todo
    return 0;
  }
};