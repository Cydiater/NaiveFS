#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <optional>
#include <vector>

#include "nfs/config.hpp"
#include "nfs/disk.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/utils.hpp"

/*
  [block_addr: uint32_t, inode_idx: uint32_t, block_offset: uint32_t]
*/

struct SegmentSummary {
  static constexpr uint32_t INVALID_ENTRY = 0;
  static constexpr uint32_t MAX_ENTRIES = kSummarySize / 12;

  uint32_t occupied;
  uint32_t entries[MAX_ENTRIES][3];

  uint32_t count() const {
    for (uint32_t i = 0; i < MAX_ENTRIES; i++) {
      if (entries[i][0] == INVALID_ENTRY)
        return i;
    }
    return MAX_ENTRIES;
  }
};

class SegmentBuilder {
  char *buf_;
  SegmentSummary *summary_;
  uint32_t offset_;
  uint32_t cursor_;
  Disk *disk_;

public:
  SegmentBuilder(Disk *disk)
      : offset_(kSummarySize), cursor_(kCRSize), disk_(disk) {
    buf_ = disk->align_alloc(kSegmentSize);
    summary_ = reinterpret_cast<SegmentSummary *>(buf_);
  }
  ~SegmentBuilder() { free(buf_); }

  void seek(const uint32_t cursor) {
    debug("SegmentBuidler: seek to " + std::to_string(cursor));
    offset_ = kSummarySize;
    cursor_ = cursor;
  }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    if (offset >= cursor_ && offset < cursor_ + kSegmentSize) {
      assert(offset - cursor_ + size <= kSegmentSize);
      std::memcpy(buf, buf_ + offset - cursor_, size);
      return;
    }
    disk_->read(buf, offset, size);
  }

  std::optional<uint32_t> push(const char *this_buf) {
    debug("SegmentBuilder: push block at offset = " + std::to_string(offset_));
    if (offset_ + kBlockSize >= kSegmentSize)
      return std::nullopt;
    std::memcpy(buf_ + offset_, this_buf, kBlockSize);
    auto ret = cursor_ + offset_;
    offset_ += kBlockSize;
    return ret;
  }

  std::optional<uint32_t> push(const DiskInode *disk_inode) {
    debug("SegmentBuilder: push inode at offset = " + std::to_string(offset_));
    auto inc = sizeof(DiskInode);
    if (offset_ + inc >= kSegmentSize)
      return std::nullopt;
    std::memcpy(buf_ + offset_, disk_inode, inc);
    auto ret = cursor_ + offset_;
    offset_ += inc;
    return ret;
  }

  std::pair<const char *, uint32_t> build() {
    // todo: build imap and summary
    summary_->occupied = 1;
    return {buf_, cursor_};
  }
};

class SegmentsManager {
  Disk *disk_;
  std::unique_ptr<SegmentBuilder> builder_;
  SegmentSummary current_summary_;

  uint32_t find_next_empty(uint32_t cursor) {
    // todo: consider warping
    disk_->nread(reinterpret_cast<char *>(&current_summary_), cursor,
                 sizeof(SegmentSummary));
    if (current_summary_.occupied == 0) {
      return cursor;
    }
    return find_next_empty(cursor + kSegmentSize);
  }

public:
  SegmentsManager(Disk *disk)
      : disk_(disk), builder_(std::make_unique<SegmentBuilder>(disk)) {
    builder_->seek(find_next_empty(kCRSize));
  }

  void flush() {
    auto [buf, offset] = builder_->build();
    disk_->write(buf, offset, kSegmentSize);
    auto next_segment_addr = find_next_empty(offset + kSegmentSize);
    builder_->seek(next_segment_addr);
  }

  template <typename obj_t> uint32_t push(obj_t obj) {
    auto pushed = builder_->push(obj);
    if (pushed == std::nullopt) {
      flush();
      return push(obj);
    }
    return pushed.value();
  }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    builder_->read(buf, offset, size);
  }
};