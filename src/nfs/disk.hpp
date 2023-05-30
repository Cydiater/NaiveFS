#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "nfs/config.hpp"

class MemDisk {
  char *mem_;
  uint32_t gb_;

public:
  MemDisk(const char *, const int gb) : gb_(gb) {
    mem_ = new char[gb * 1024 * 1024 * 1024];
  }

  ~MemDisk() { delete[] mem_; }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    assert(offset + size < gb_ * 1024 * 1024);
    std::memcpy(buf, mem_ + offset, size);
  }

  void write(const char *buf, const uint32_t offset, const uint32_t size) {
    assert(offset + size < gb_ * 1024 * 1024);
    std::memcpy(mem_ + offset, buf, size);
  }
};

using Disk = MemDisk;