#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <unistd.h>

#include "nfs/config.hpp"
#include "nfs/utils.hpp"

/*class MemDisk {
  char *mem_;

public:
  MemDisk(const char *, const int) {
    mem_ = new char[kMemDiskCapacityMB * 1024 * 1024];
    std::memset(mem_, 0, kMemDiskCapacityMB * 1024 * 1024);
  }

  ~MemDisk() { delete[] mem_; }

  uint32_t end() const { return kMemDiskCapacityMB * 1024 * 1024; }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    debug("Disk read [" + std::to_string(offset) + ", " +
          std::to_string(offset + size) + ")");
    assert(offset + size <= kMemDiskCapacityMB * 1024 * 1024);
    std::memcpy(buf, mem_ + offset, size);
  }

  void write(const char *buf, const uint32_t offset, const uint32_t size) {
    debug("Disk write [" + std::to_string(offset) + ", " +
          std::to_string(offset + size) + ")");
    assert(offset + size < kMemDiskCapacityMB * 1024 * 1024);
    std::memcpy(mem_ + offset, buf, size);
  }
};*/

class Disk {
  int fd;

public:
  Disk(const char *_path, const uint32_t capacity) {
    debug("Disk " + std::string(_path));
    fd = open(_path, O_CREAT | O_DIRECT | O_NOATIME | O_RDWR, 0666);
    debug(strerror(errno));
    assert(fd != -1);
    debug("Disk size " + std::to_string(capacity * 1024 * 1024 * 1024));
    auto res = ftruncate(fd, capacity * 1024 * 1024 * 1024);
    assert(res != -1);
  }

  ~Disk() { close(fd); }

  static char *align_alloc(uint32_t size) {
    char *buf;
    posix_memalign(reinterpret_cast<void **>(&buf), 512, size);
    return buf;
  }

  uint32_t end() const { return kDiskCapacityGB * 1024 * 1024 * 1024; }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    if (size <= 4 * kBlockSize) {
      nread(buf, offset, size);
      return;
    }
    debug("Disk read [" + std::to_string(offset) + ", " +
          std::to_string(offset + size) + ")");
    assert(offset + size <= end());
    assert((size_t)buf % 512 == 0);
    assert(offset % 512 == 0);
    auto res = pread(fd, buf, size, offset);
    assert(res != -1);
  }
  void nread(char *buf, const uint32_t offset, const uint32_t size) {
    debug("Disk nread [" + std::to_string(offset) + ", " +
          std::to_string(offset + size) + ")");
    assert(offset + size <= end());
    uint32_t loffset = offset / 512 * 512;
    uint32_t rsize = ((size + (offset - loffset)) + 511) / 512 * 512;
    char *newbuf;
    posix_memalign(reinterpret_cast<void **>(&newbuf), 512, rsize);
    auto res = pread(fd, newbuf, rsize, loffset);
    assert(res != -1);
    memcpy(buf, newbuf + (offset - loffset), size);
  }

  void write(const char *buf, const uint32_t offset, const uint32_t size) {
    debug("Disk write [" + std::to_string(offset) + ", " +
          std::to_string(offset + size) + ")");
    assert(offset + size <= end());
    assert((size_t)buf % 512 == 0);
    assert(size % 512 == 0);
    assert(offset % 512 == 0);
    auto res = pwrite(fd, buf, size, offset);
    assert(res != -1);
  }
};