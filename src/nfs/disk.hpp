#pragma once

#include <string>

#include "nfs/config.hpp"

class Disk {
  int fd_;
  bool initialized_;

public:
  Disk(const char *disk_path, const int gb) {
    // todo
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    // todo
  }

  void write(const char *buf, uint32_t offset, uint32_t size) {
    // todo
  }
};