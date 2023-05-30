#pragma once

#include <cstring>
#include <memory>
#include <optional>

#include <sys/stat.h>
#include <unistd.h>

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/imap.hpp"
#include "nfs/seg.hpp"

class Inode {
  std::unique_ptr<DiskInode> disk_inode_;
  SegmentBuilder *seg_;
  Imap *imap_;

public:
  Inode(std::unique_ptr<DiskInode> disk_inode, SegmentBuilder *seg, Imap *imap)
      : disk_inode_(std::move(disk_inode)), seg_(seg), imap_(imap) {}

  void read(char *buf, uint32_t offset, uint32_t size) {
    // todo
  }

  std::optional<uint32_t> find(const std::string &name) {
    // todo
    return std::nullopt;
  }
}