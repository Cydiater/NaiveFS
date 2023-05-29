#pragma once

#include <memory>
#include <optional>

#include "nfs/disk.hpp"
#include "nfs/fd.hpp"
#include "nfs/id.hpp"
#include "nfs/imap.hpp"
#include "nfs/inode.hpp"
#include "nfs/seg.hpp"
#include "nfs/utils.hpp"

class NoEntry : public std::exception {
public:
  char *what() { return "No entry"; }
};

class NaiveFS {
  std::unique_ptr<SegmentsManager> seg_mgr_;
  std::unique_ptr<SegmentBuilder> seg_builder_;
  std::unique_ptr<FDManager> fd_mgr_;
  std::unique_ptr<IDManager> id_mgr_;

public:
  NaiveFS() {}

  void readdir() {
    // todo
  }

  void read(const uint32_t fd, char *buf, uint32_t offset, uint32_t size) {
    // todo
  }

  std::optional<uint32_t> open(const char *path, const int flags) {
    auto inode_idx = id_mgr_->root_inode_idx;
    auto inode = get_inode(inode_idx);
    const auto &[path_components, name] = parse_path_components(path);
    for (const auto &com : path_components) {
      auto found = inode->find(com);
      if (!found.has_value()) {
        throw NoEntry();
      }
      inode_idx = found.value();
      inode = get_inode(inode_idx);
    }
    auto found = inode->find(name);
    if (!found.has_value()) {
      // create inode
      // add file to parent dir
      // add value to found
    }
    inode_idx = found.value();
    auto fd = fd_mgr_->allocate(inode_idx);
    return fd;
  }

private:
  std::unique_ptr<Inode> get_inode(const uint32_t inode_idx) {
    auto inode_addr = Imap::instance()->get(inode_idx);
    auto inode = std::make_unique<Inode>();
    seg_builder_->read(reinterpret_cast<char *>(inode.get()), inode_addr,
                       sizeof(Inode));
    return inode;
  }
};