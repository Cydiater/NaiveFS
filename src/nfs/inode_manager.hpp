#pragma once

#include <atomic>
#include <memory>

#include "nfs/config.hpp"
#include "nfs/imap.hpp"

class InodeManager {
  std::atomic<uint32_t> cur_idx;
  std::unique_ptr<Imap> imap_;

public:
  static constexpr uint32_t root_inode_idx = 0;

  InodeManager() { cur_idx = root_inode_idx; }

  int allocate() { return ++cur_idx; }
};