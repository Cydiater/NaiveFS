#pragma once

#include <cstdint>
#include <string>

constexpr uint32_t kCRFlushingSeconds = 30;
constexpr const char *kDiskPath = "/tmp/disk";
constexpr uint32_t kMaxInode = 65536;
constexpr uint32_t kFreeSegmentsUpperbound = 128;
constexpr uint32_t kNumMergingSegments = 32;
constexpr uint32_t kBlockSize = 4 * 1024;
constexpr uint32_t kSegmentSize = 512 * 1024;
constexpr uint32_t kSummarySize = 2048;
constexpr uint32_t kCRImapSize = kMaxInode * 4 + 512;

// for MemDisk
constexpr uint32_t kMemDiskCapacityMB = 16;

#ifdef SMALL_DISK

constexpr uint32_t kInodeDirectCnt = 24;
constexpr uint32_t kGCCheckSeconds = 5;
constexpr uint32_t kDiskCapacityMB = 128;
constexpr uint32_t kFreeSegmentsLowerbound = 128;

#else

constexpr uint32_t kInodeDirectCnt = 24;
constexpr uint32_t kGCCheckSeconds = 1;
constexpr uint32_t kDiskCapacityMB = 2 * 1024;
constexpr uint32_t kFreeSegmentsLowerbound = 32;

#endif

constexpr uint32_t kMaxSegments =
    kDiskCapacityMB * 1024 / (kSegmentSize / 1024);
constexpr uint32_t kCRSize = kCRImapSize + kMaxSegments * 8;