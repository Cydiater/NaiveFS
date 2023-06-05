#pragma once

#include <atomic>
#include <cassert>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <vector>

#include "nfs/config.hpp"
#include "nfs/disk.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/imap.hpp"
#include "nfs/utils.hpp"

/*
  [block_addr: uint32_t, inode_idx: uint32_t, block_offset: uint32_t]
*/

struct SegmentSummary {
  static constexpr uint32_t INVALID_ENTRY = 0;
  static constexpr uint32_t MAX_ENTRIES = kSummarySize / 12 - 1;

  uint32_t len_imap_;
  uint32_t _pad[2];
  uint32_t entries[MAX_ENTRIES][3];

  void for_each_entry(
      std::function<void(const uint32_t, const uint32_t, const uint32_t)>
          callback) {
    for (uint32_t i = 0; i < MAX_ENTRIES; i++) {
      if (entries[i][0] == INVALID_ENTRY)
        break;
      callback(entries[i][0], entries[i][1], entries[i][2]);
    }
  }
};

class SegmentBuilder {
  char *buf_;
  SegmentSummary *summary_;
  uint32_t offset_;
  uint32_t cursor_;
  uint32_t occupied_bytes_;
  uint32_t block_cnt_;
  Disk *disk_;
  std::vector<std::pair<uint32_t /* inode_idx */, uint32_t /* addr */>> imap_;

public:
  SegmentBuilder(Disk *disk)
      : offset_(kSummarySize), cursor_(kCRSize), disk_(disk) {
    buf_ = disk->align_alloc(kSegmentSize);
    summary_ = reinterpret_cast<SegmentSummary *>(buf_);
    block_cnt_ = 0;
    imap_.clear();
  }
  ~SegmentBuilder() { free(buf_); }

  uint32_t imap_size() const { return imap_.size() * 8; }

  void seek(const uint32_t cursor) {
    debug("SegmentBuidler: seek to " + std::to_string(cursor));
    offset_ = kSummarySize;
    cursor_ = cursor;
    occupied_bytes_ = 0;
    block_cnt_ = 0;
  }

  void read(char *buf, const uint32_t offset, const uint32_t size) {
    if (offset >= cursor_ && offset < cursor_ + kSegmentSize) {
      assert(offset - cursor_ + size <= kSegmentSize);
      std::memcpy(buf, buf_ + offset - cursor_, size);
      return;
    }
    disk_->read(buf, offset, size);
  }

  std::optional<uint32_t>
  push(const std::tuple<char *, uint32_t /* inode_idx */,
                        uint32_t /* inode_offset */>
           block) {
    if (offset_ + kBlockSize + imap_size() > kSegmentSize)
      return std::nullopt;
    std::memcpy(buf_ + offset_, std::get<0>(block), kBlockSize);
    auto ret = cursor_ + offset_;
    offset_ += kBlockSize;
    occupied_bytes_ += kBlockSize;
    summary_->entries[block_cnt_][0] = ret;
    summary_->entries[block_cnt_][1] = std::get<1>(block);
    summary_->entries[block_cnt_][2] = std::get<2>(block);
    return ret;
  }

  std::optional<uint32_t>
  push(const std::pair<DiskInode *, uint32_t /* inode_idx */> inode) {
    auto inc = sizeof(DiskInode);
    if (offset_ + inc + imap_size() + 8 > kSegmentSize)
      return std::nullopt;
    std::memcpy(buf_ + offset_, std::get<0>(inode), inc);
    auto ret = cursor_ + offset_;
    offset_ += inc;
    occupied_bytes_ += inc;
    imap_.push_back({ret, std::get<1>(inode)});
    return ret;
  }

  /*
    return: [buffer, offset, occupied_bytes]
   */
  std::tuple<const char *, uint32_t, uint32_t> build() {
    auto ptr = buf_ + kSegmentSize;
    uint32_t len = imap_.size();
    summary_->len_imap_ = len;
    for (uint32_t i = 0; i < len; i++) {
      ptr -= 8;
      std::memcpy(ptr, &imap_[i].first, 4);
      std::memcpy(ptr + 4, &imap_[i].second, 4);
    }
    return {buf_, cursor_, occupied_bytes_};
  }
};

class SegmentsManager {
  Disk *disk_;
  std::unique_ptr<SegmentBuilder> builder_;
  Imap *imap_;
  std::shared_mutex lock_seg_status_;

  struct SegmentStatus {
    uint32_t flushing_version;
    uint32_t occupied_bytes;
  } * seg_status_;
  std::atomic<uint32_t> free_segments_;

  uint32_t find_next_empty(uint32_t cursor) {
    while (true) {
      auto idx = (cursor - kCRSize) / kSegmentSize;
      if (seg_status_[idx].occupied_bytes == 0) {
        return cursor;
      }
      cursor += kSegmentSize;
      // todo: consider warping
    }
  }

  static constexpr uint32_t get_size(const char *) { return kBlockSize; }

  static constexpr uint32_t get_size(const DiskInode *) {
    return sizeof(DiskInode);
  }

public:
  SegmentsManager(Disk *disk, Imap *imap, char *from)
      : disk_(disk), builder_(std::make_unique<SegmentBuilder>(disk)),
        imap_(imap), seg_status_(reinterpret_cast<SegmentStatus *>(from)) {
    free_segments_ = 0;
    for (uint32_t i = 0; i < kMaxSegments; i++) {
      free_segments_ += seg_status_[i].occupied_bytes == 0;
    }
    builder_->seek(find_next_empty(kCRSize));
  }

  ~SegmentsManager() { delete[] seg_status_; }

  std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>
  select_segments_for_gc() {
    if (free_segments_ >= kFreeSegmentsLowerbound)
      return {};
    auto lock = std::shared_lock(lock_seg_status_);
    std::vector<uint32_t> candidate_seg_indices;
    auto cmp = [&](const uint32_t lhs, const uint32_t rhs) {
      if (seg_status_[lhs].occupied_bytes == 0)
        return false;
      if (seg_status_[rhs].occupied_bytes == 0)
        return true;
      auto lhs_value =
          seg_status_[lhs].occupied_bytes * seg_status_[lhs].flushing_version;
      auto rhs_value =
          seg_status_[rhs].occupied_bytes * seg_status_[rhs].flushing_version;
      return lhs_value < rhs_value;
    };
    std::set<uint32_t, decltype(cmp)> heap;
    for (uint32_t i = 0; i < kMaxSegments; i++) {
      heap.insert(i);
      if (heap.size() > kNumMergingSegments)
        heap.erase(--heap.end());
    }
    std::map<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>
        ds_by_inode_idx;
    candidate_seg_indices = std::vector<uint32_t>{heap.begin(), heap.end()};
    auto seg_buf = Disk::align_alloc(kSummarySize);
    auto summary = reinterpret_cast<SegmentSummary *>(seg_buf);
    for (auto seg_idx : candidate_seg_indices) {
      auto addr = kCRSize + seg_idx * kSegmentSize;
      disk_->read(seg_buf, addr, kSummarySize);
      summary->for_each_entry([&](const uint32_t addr, const uint32_t inode_idx,
                                  const uint32_t code) {
        ds_by_inode_idx[inode_idx].push_back({addr, code});
      });
    }
    delete[] seg_buf;
    return ds_by_inode_idx;
  }

  const char *get_buf() { return reinterpret_cast<const char *>(seg_status_); }

  static uint32_t addr2segidx(const uint32_t addr) {
    assert(addr >= kCRSize);
    assert(addr < kDiskCapacityGB * 1024 * 1024 * 1024 - kCRSize);
    return (addr - kCRSize) / kSegmentSize;
  }

  void flush() {
    auto [buf, offset, occupied_bytes] = builder_->build();
    auto idx = (offset - kCRSize) / kSegmentSize;
    seg_status_[idx].occupied_bytes = occupied_bytes;
    seg_status_[idx].flushing_version = imap_->version();
    disk_->write(buf, offset, kSegmentSize);
    auto next_segment_addr = find_next_empty(offset + kSegmentSize);
    builder_->seek(next_segment_addr);
  }

  template <typename obj_t> uint32_t push(obj_t obj, const uint32_t old_addr) {
    auto lock = std::unique_lock(lock_seg_status_);
    if (old_addr == DiskInode::INVALID_ADDR)
      return push(obj);
    auto idx = addr2segidx(old_addr);
    auto new_addr = push(obj);
    if (addr2segidx(new_addr) == idx)
      return new_addr;
    assert(seg_status_[idx].occupied_bytes >= get_size(std::get<0>(obj)));
    seg_status_[idx].occupied_bytes -= get_size(std::get<0>(obj));
    if (seg_status_[idx].occupied_bytes == 0) {
      free_segments_ += 1;
    }
    return new_addr;
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