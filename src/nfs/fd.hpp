#pragma once

#include <map>
#include <optional>

#include "nfs/config.hpp"

class FDManager {
  std::map<uint32_t, uint32_t> fd2id;

public:
  uint32_t allocate(const uint32_t inode_idx) {
    // todo
    return 0;
  }

  std::optional<uint32_t> get(const uint32_t fd) {
    auto it = fd2id.find(fd);
    if (it == fd2id.end())
      return std::nullopt;
    return it->second;
  }
};