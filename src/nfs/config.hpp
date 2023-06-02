#pragma once

#include <cstdint>
#include <string>

#ifdef SMALL_SEGMENT

// 用于测试 SegmentsManager

constexpr uint32_t kSegmentSize = 1024;
constexpr uint32_t kBlockSize = 256;
constexpr uint32_t kSummarySize = 128;

constexpr uint32_t kInodeDirectCnt = 24;
constexpr const char *kDiskPath = "/tmp/disk";
constexpr uint32_t kDiskCapacityGB = 16;
constexpr uint32_t kMaxInode = 65536;
constexpr uint32_t kCRSize = kMaxInode * 4;
constexpr uint32_t kMemDiskCapacityMB = 16;

#else

constexpr uint32_t kInodeDirectCnt = 24;
constexpr const char *kDiskPath = "/tmp/disk";
constexpr uint32_t kDiskCapacityGB = 16;
constexpr uint32_t kBlockSize = 4 * 1024;
constexpr uint32_t kMaxInode = 65536;
constexpr uint32_t kCRSize = kMaxInode * 4;
constexpr uint32_t kSegmentSize = 512 * 1024;
constexpr uint32_t kSummarySize = 1024;
constexpr uint32_t kMemDiskCapacityMB = 16;

#endif