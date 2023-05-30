#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <vector>

#include "nfs/config.hpp"
#include "nfs/disk.hpp"
#include "nfs/disk_inode.hpp"

class SegmentBuilder {
  char *buf_;
  uint32_t offset_;
  uint32_t cursor_;
  Disk *disk_;

public:
  SegmentBuilder(Disk *disk) : disk_(disk), cursor_(kSegmentSize) {
    buf_ = new char[kSegmentSize];
  }
  ~SegmentBuilder() { delete[] buf_; }

  void seek(const uint32_t offset) {
    assert(cursor_ == kSummarySize);
    offset_ = offset;
  }

  void read(char *buf, uint32_t offset, uint32_t size) {
    if (offset >= offset_ && offset < offset_ + kSegmentSize) {
      // do local read
      return;
    }
    disk_->read(buf, offset, size);
  }

  void push(const char *) {
    // todo
  }

  void push(const DiskInode *disk_inode) {
    auto inc = sizeof(DiskInode);
    std::memcpy(buf_ + offset_, disk_inode, inc);
    offset_ += inc;
  }
};

class SegmentsManager {
public:
  void push(const char *buf) {
    // todo
  }
};