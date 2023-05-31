#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "nfs/config.hpp"

class MemDisk {
  char *mem_;

public:
  MemDisk(const char *, const int) {
    mem_ = new char[kMemDiskCapacityMB * 1024 * 1024];
    std::memset(mem_, 0, kMemDiskCapacityMB * 1024 * 1024);
  }

  ~MemDisk() { delete[] mem_; }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    assert(offset + size < kMemDiskCapacityMB * 1024 * 1024);
    std::memcpy(buf, mem_ + offset, size);
  }

  void write(const char *buf, const uint32_t offset, const uint32_t size) {
    assert(offset + size < kMemDiskCapacityMB * 1024 * 1024);
    std::memcpy(mem_ + offset, buf, size);
  }
};

using Disk = MemDisk;