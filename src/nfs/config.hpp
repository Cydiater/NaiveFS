#pragma once

#include <cstdint>
#include <string>

constexpr uint32_t kCRFlushingSeconds = 30;
constexpr uint32_t kGCCheckSeconds = 1;
constexpr uint32_t kInodeDirectCnt = 24;
constexpr const char *kDiskPath = "/tmp/disk";
constexpr uint32_t kDiskCapacityGB = 2;
constexpr uint32_t kMaxInode = 65536;
constexpr uint32_t kMemDiskCapacityMB = 16;

#ifdef SMALL_SEGMENT

// 用于测试 SegmentsManager

constexpr uint32_t kSegmentSize = 1024;
constexpr uint32_t kBlockSize = 256;
constexpr uint32_t kSummarySize = 128;

#else

constexpr uint32_t kBlockSize = 4 * 1024;
constexpr uint32_t kSegmentSize = 512 * 1024;
constexpr uint32_t kSummarySize = 2048;
constexpr uint32_t kMaxSegments =
    kDiskCapacityGB * 1024 * 1024 / (kSegmentSize / 1024);
constexpr uint32_t kCRImapSize = kMaxInode * 4 + 512;
constexpr uint32_t kCRSize = kCRImapSize + kMaxSegments * 8;

#endif