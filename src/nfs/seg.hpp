#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <vector>

#include "nfs/config.hpp"
#include "nfs/disk.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/utils.hpp"

class SegmentBuilder {
  char *buf_;
  uint32_t offset_;
  uint32_t cursor_;
  Disk *disk_;

public:
  SegmentBuilder(Disk *disk) : offset_(kSummarySize), cursor_(kCRSize), disk_(disk) {
    buf_ = new char[kSegmentSize];
  }
  ~SegmentBuilder() { delete[] buf_; }

  void seek(const uint32_t cursor) {
    assert(offset_ == kSummarySize);
    cursor_ = cursor;
  }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    if (offset >= cursor_ && offset < cursor_ + kSegmentSize) {
      debug("local read offset = "   + std::to_string(offset) +
            " size = " + std::to_string(size));
      assert(offset - cursor_ + size <= kSegmentSize);
      std::memcpy(buf, buf_ + offset - cursor_, size);
      return;
    }
    debug("disk read offset =  " + std::to_string(offset) +
          " size = " + std::to_string(size) + " cursor_ = " + std::to_string(cursor_));
    disk_->read(buf, offset, size);
  }

  uint32_t push(const char *this_buf) {
    debug("Push Block " + std::to_string(offset_));
    std::memcpy(buf_ + offset_, this_buf, kBlockSize);
    auto ret = cursor_ + offset_;
    offset_ += kBlockSize;
    assert(offset_ < kSegmentSize);
    return ret;
  }

  uint32_t push(const DiskInode *disk_inode) {
    debug("Push Inode " + std::to_string(offset_));
    auto inc = sizeof(DiskInode);
    std::memcpy(buf_ + offset_, disk_inode, inc);
    auto ret = cursor_ + offset_;
    offset_ += inc;
    assert(offset_ < kSegmentSize);
    return ret;
  }
};

class SegmentsManager {
public:
  void push(const char *buf) {
    // todo
  }
};