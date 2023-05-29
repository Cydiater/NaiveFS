#pragma once

#include "nfs/config.hpp"

#include <vector>

class SegmentBuilder {
  char *buf_;
  uint32_t offset_;

public:
  SegmentBuilder() { buf_ = new char[kSegmentSize]; }
  ~SegmentBuilder() { delete[] buf_; }

  void read(char *buf, uint32_t offset, uint32_t size) {
    if (offset >= offset_ && offset < offset_ + kSegmentSize) {
      // do local read
      return;
    }
    Disk::instance()->read(buf, offset, size);
  }

  void push(const char *buf) {
    // todo
  }

  void push(const Inode *inode) {
    // todo
  }
};

class SegmentsManager {
public:
  void push(const char *buf) {
    // todo
  }
};