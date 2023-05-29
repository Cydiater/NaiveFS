#pragma once

#include <cstdint>
#include <string>

constexpr uint32_t kInodeDirectCnt = 24;
constexpr char *kDiskPath = "/tmp/disk";
constexpr uint32_t kDiskCapacityGB = 16;
constexpr uint32_t kBlockSize = 4 * 1024;
constexpr uint32_t kMaxInode = 4096;
constexpr uint32_t kCRSize = kMaxInode * 4;
constexpr uint32_t kSegmentSize = 512 * 1024;