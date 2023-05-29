#pragma once

#include <memory>
#include <optional>

#include "nfs/fd.hpp"
#include "nfs/seg.hpp"

class NaiveFS {
  std::unique_ptr<SegmentsManager> seg_mgr_;
  std::unique_ptr<SegmentBuilder> seg_builder_;
  std::unique_ptr<FDManager> fd_mgr_;

public:
  NaiveFS() {}

  std::optional<uint32_t> open(const char *path, const int flags) {
    // todo
  }
};