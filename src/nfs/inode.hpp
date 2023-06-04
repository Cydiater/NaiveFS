#pragma once

#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/imap.hpp"
#include "nfs/seg.hpp"
#include "nfs/utils.hpp"

class Inode {
  std::unique_ptr<DiskInode> disk_inode_;
  SegmentsManager *seg_;
  bool dirty_;

  /*
    callback:
      - arg0: 传入块的地址
      - arg1: 在块内的偏移
      - arg2: 在块内相关的字节数
      - return: 这个块下一个版本的地址
  */

  void for_each_block(
      uint32_t offset, const uint32_t size,
      std::function<uint32_t(const uint32_t, const uint32_t, const uint32_t)>
          callback) {
    assert(dirty_ == false);
    int dirty_1 = -1;
    auto end = offset + size;
    uint32_t *indirect1 = nullptr;
    uint32_t indirect1_addr = -1; // an impossible value
    uint32_t *indirect21 = nullptr;
    uint32_t indirect21_addr = -1; // an impossible value
    uint32_t *indirect22 = nullptr;
    uint32_t indirect22_addr = -1;
    if (end > disk_inode_->size) {
      disk_inode_->size = end;
      dirty_ = true;
    }
    while (offset < end) {
      const auto [i0, i1, i2] = DiskInode::translate(offset);
      debug("offset = " + std::to_string(offset) +
            " i0 = " + std::to_string(i0) + " i1 = " + std::to_string(i1) +
            " i2 = " + std::to_string(i2));
      const auto next_offset = (offset / kBlockSize + 1) * kBlockSize;
      const auto this_size = std::min(next_offset - offset, end - offset);
      if (i0 < kInodeDirectCnt) {
        auto this_addr = disk_inode_->directs[i0];
        auto new_addr = callback(this_addr, offset % kBlockSize, this_size);
        if (new_addr != this_addr) {
          dirty_ = true;
          debug("set disk_inode directs " + std::to_string(i0) + " -> " +
                std::to_string(new_addr));
          disk_inode_->directs[i0] = new_addr;
        }
      } else if (i0 < kInodeDirectCnt + 1) {
        if (indirect1_addr != disk_inode_->indirect1) {
          if (indirect1 == nullptr) {
            indirect1 = new uint32_t[kBlockSize / 4];
          }
          indirect1_addr = disk_inode_->indirect1;
          if (indirect1_addr != 0)
            seg_->read(reinterpret_cast<char *>(indirect1), indirect1_addr,
                       kBlockSize);
          else
            memset(indirect1, 0, kBlockSize);
        }
        auto this_addr = indirect1[i1];
        auto new_addr = callback(this_addr, offset % kBlockSize, this_size);
        if (new_addr != this_addr) {
          dirty_ = true;
          indirect1[i1] = new_addr;
        }
      } else {
        debug("indirect2 " + std::to_string(disk_inode_->indirect2));
        if (indirect22_addr != disk_inode_->indirect2) {
          if (indirect22 == nullptr) {
            indirect22 = new uint32_t[kBlockSize / 4];
          }
          indirect22_addr = disk_inode_->indirect2;
          indirect21_addr = -1;
          if (indirect22_addr != 0)
            seg_->read(reinterpret_cast<char *>(indirect22), indirect22_addr,
                       kBlockSize);
          else
            memset(indirect22, 0, kBlockSize);
        }
        debug("indirect1 " + std::to_string(indirect22[i2]));
        if (indirect21_addr != indirect22[i2]) {
          debug("ok changed " + std::to_string(i2));
          if (indirect21 != nullptr) {
            if (dirty_1 != -1) {
              auto addr = seg_->push(reinterpret_cast<const char *>(indirect21),
                                     indirect22[dirty_1]);
              indirect22[dirty_1] = addr;
              dirty_1 = -1;
            }
          } else {
            indirect21 = new uint32_t[kBlockSize / 4];
          }
          indirect21_addr = indirect22[i2];
          if (indirect21_addr != 0)
            seg_->read(reinterpret_cast<char *>(indirect21), indirect21_addr,
                       kBlockSize);
          else
            memset(indirect21, 0, kBlockSize);
        }
        debug("i2Block " + std::to_string(indirect21[i1]));
        auto this_addr = indirect21[i1];
        auto new_addr = callback(this_addr, offset % kBlockSize, this_size);
        if (new_addr != this_addr) {
          dirty_ = true;
          dirty_1 = i2; // which offset of indirect22 it comes from
          indirect21[i1] = new_addr;
        }
      }
      offset = next_offset;
    }
    if (indirect1 != nullptr) {
      if (dirty_) {
        auto addr = seg_->push(reinterpret_cast<const char *>(indirect1),
                               disk_inode_->indirect1);
        disk_inode_->indirect1 = addr;
      }
      delete[] indirect1;
    }
    if (indirect21 != nullptr) {
      if (dirty_1 != -1) {
        auto addr = seg_->push(reinterpret_cast<const char *>(indirect21),
                               indirect22[dirty_1]);
        debug("write back 21 " + std::to_string(addr));
        indirect22[dirty_1] = addr;
      }
      delete[] indirect21;
    }
    if (indirect22 != nullptr) {
      if (dirty_) {
        auto addr = seg_->push(reinterpret_cast<const char *>(indirect22),
                               disk_inode_->indirect2);
        debug("write back 22 " + std::to_string(addr));
        disk_inode_->indirect2 = addr;
      }
      delete[] indirect22;
    }
  }

  /*
    callback:
      - arg0: 目录的名字
      - arg1: 目录的 inode_idx
      - arg2: 目录在 inode 内的偏移量
      - return: 是否完成
  */

  void for_each_entry_once(std::function<bool(const std::string &name,
                                              const uint32_t, const uint32_t)>
                               callback) {
    // assume that directory should not be very large here
    // fix: support large directory
    assert(disk_inode_->size <= kBlockSize * 3);
    if (disk_inode_->size == 0)
      return;
    debug("disk_inode_->size = " + std::to_string(disk_inode_->size));
    auto buf = new char[disk_inode_->size];
    read(buf, 0, disk_inode_->size);
    uint32_t offset = 0;
    while (offset < disk_inode_->size) {
      auto this_offset = offset;
      const auto [this_name, this_inode_idx, this_deleted] =
          parse_one_dir_entry(buf, offset);
      if (this_deleted)
        continue;
      auto done = callback(this_name, this_inode_idx, this_offset);
      if (done) {
        break;
      }
    }
    delete[] buf;
  }

public:
  Inode(std::unique_ptr<DiskInode> disk_inode, SegmentsManager *seg)
      : disk_inode_(std::move(disk_inode)), seg_(seg), dirty_(false) {}

  std::unique_ptr<DiskInode> downgrade() {
    assert(disk_inode_ != nullptr);
    debug(std::to_string(disk_inode_->directs[0]));
    auto ret = std::unique_ptr<DiskInode>(nullptr);
    disk_inode_.swap(ret);
    return ret;
  }

  std::unique_ptr<DiskInode> write(char *buf, uint32_t offset, uint32_t size) {
    debug("Inode write " + std::to_string(offset) + " " + std::to_string(size) +
          " " + std::to_string(disk_inode_->size));
    assert(offset <= disk_inode_->size);
    for_each_block(offset, size,
                   [&buf, this](const uint32_t addr, const uint32_t this_offset,
                                const uint32_t this_size) {
                     if (this_size == kBlockSize) {
                       assert(this_offset == 0);
                       auto new_addr = seg_->push(buf, addr);
                       buf += kBlockSize;
                       return new_addr;
                     }
                     auto this_buf = Disk::align_alloc(kBlockSize);
                     if (addr >= kCRSize)
                       seg_->read(this_buf, addr, kBlockSize);
                     std::memcpy(this_buf + this_offset, buf, this_size);
                     auto new_addr = seg_->push(this_buf, addr);
                     delete[] this_buf;
                     buf += this_size;
                     return new_addr;
                   });
    return downgrade();
  }

  uint32_t read(char *buf, uint32_t offset, uint32_t size) {
    debug("Inode read " + std::to_string(offset) + " " + std::to_string(size) +
          " " + std::to_string(disk_inode_->size));
    if (offset >= disk_inode_->size)
      return 0;
    uint32_t actual_read = 0;
    size = std::min(size, disk_inode_->size - offset);
    for_each_block(offset, size,
                   [&buf, &actual_read, this](const uint32_t addr,
                                              const uint32_t this_offset,
                                              const uint32_t this_size) {
                     seg_->read(buf, addr + this_offset, this_size);
                     buf += this_size;
                     actual_read += this_size;
                     return addr;
                   });
    return actual_read;
  }

  std::unique_ptr<DiskInode> push(const std::string &name,
                                  const uint32_t inode_idx) {
    debug("inode dir push entry size =  " + std::to_string(disk_inode_->size) +
          " " + name + " -> " + std::to_string(inode_idx));
    auto [buf, len] = make_one_dir_entry(name, inode_idx);
    return write(buf, disk_inode_->size, len);
  }

  std::vector<std::string> list_entries() {
    std::vector<std::string> names;
    for_each_entry_once(
        [&names](const std::string &this_name, const uint32_t, const uint32_t) {
          names.push_back(this_name);
          return false;
        });
    return names;
  }

  std::unique_ptr<DiskInode> erase_entry(const std::string &name) {
    std::unique_ptr<DiskInode> ret = nullptr;
    for_each_entry_once([this, &name, &ret](const std::string &this_name,
                                            const uint32_t this_inode_idx,
                                            const uint32_t offset) {
      if (name == this_name) {
        auto [buf, len] = make_one_dir_entry(this_name, this_inode_idx);
        buf[len - 1] = true; /* deleted = true */
        ret = write(buf, offset, len);
        return true;
      }
      return false;
    });
    return ret;
  }

  std::optional<uint32_t> find_entry(const std::string &name) {
    std::optional<uint32_t> ret = std::nullopt;
    for_each_entry_once([&name, &ret](const std::string &this_name,
                                      const uint32_t this_inode_idx,
                                      const uint32_t) {
      if (this_name == name) {
        ret = this_inode_idx;
        return true;
      }
      return false;
    });
    return ret;
  }
};