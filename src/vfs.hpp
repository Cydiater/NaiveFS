// bind fuse api w/ nfs api

#pragma once

#include <asm-generic/errno-base.h>
#include <cstdio>
#include <exception>

#include "fuse3/fuse.h"
#include "nfs/disk.hpp"
#include "nfs/utils.hpp"
#include "unistd.h"

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/nfs.hpp"

namespace vfs {

static NaiveFS nfs;

inline uint32_t get_inode_idx(const char *path, fuse_file_info *fi) {
  if (fi != nullptr && fi->fh != 0) {
    return nfs.get_inode_idx(fi->fh);
  }
  return nfs.get_inode_idx(path);
}

inline int fsync(const char *, int, struct fuse_file_info *) {
  nfs.fsync();
  return 0;
}

inline int rename(const char *old_path, const char *new_path,
                  unsigned int flags) {
  try {
    nfs.rename(old_path, new_path, flags);
  } catch (const NoEntry &e) {
    return -ENOENT;
  } catch (const DuplicateEntry &e) {
    return -EEXIST;
  }
  return 0;
}

inline int truncate(const char *path, off_t size, struct fuse_file_info *fi) {
  const auto inode_idx = get_inode_idx(path, fi);
  nfs.truncate(inode_idx, size);
  return 0;
}

inline int open(const char *path, struct fuse_file_info *fi) {
  auto fd = nfs.open(path, fi->flags);
  fi->fh = fd;
  return 0;
}

inline int create(const char *path, mode_t, struct fuse_file_info *fi) {
  return open(path, fi);
}

inline int utimens(const char *path, const struct timespec tv[2],
                   struct fuse_file_info *fi) {
  try {
    auto inode_idx = get_inode_idx(path, fi);
    debug(std::string("utimens -> inode_idx = ") + std::to_string(inode_idx));
    auto disk_inode = nfs.get_diskinode(inode_idx);
    auto access_time = tv[0].tv_sec + tv[0].tv_nsec / 1000000000.0;
    auto modify_time = tv[1].tv_sec + tv[1].tv_nsec / 1000000000.0;
    disk_inode->access_time = access_time;
    disk_inode->modify_time = modify_time;
    nfs.modify(std::move(disk_inode), inode_idx);
  } catch (std::exception &e) {
    printf("%s\n", e.what());
    // handle exceptions
    // todo: move to upper level
  }
  return 0;
}

inline int write(const char *path, const char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fi) {
  debug("FILE write: " + std::to_string(offset));
  char *tmp_buf = (char *)buf;
  nfs.write(fi->fh, tmp_buf, offset, size);
  return size;
}

inline int access(const char *, int) {
  // todo: add check here
  return F_OK;
}

inline int rmdir(const char *path) {
  try {
    nfs.unlink(path);
  } catch (const NoEntry &e) {
    return -ENOENT;
  }
  return 0;
}

inline int mkdir(const char *path, const mode_t mode) {
  try {
    nfs.mkdir(path, mode);
  } catch (const NoEntry &e) {
    return -ENOENT;
  } catch (const DuplicateEntry &e) {
    return -EEXIST;
  }
  return 0;
}

inline int read(const char *, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
  debug("FILE read: " + std::to_string(offset));
  char *tmp_buf = (char *)buf;
  return nfs.read(fi->fh, tmp_buf, offset, size);
}

inline int unlink(const char *path) {
  try {
    nfs.unlink(path);
  } catch (const NoEntry &e) {
    return -ENOENT;
  }
  return 0;
}

inline int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t,
                   fuse_file_info *, fuse_readdir_flags) {
  auto names = nfs.readdir(path);
  for (auto &name : names) {
    filler(buf, name.c_str(), nullptr, 0, (fuse_fill_dir_flags)0);
  }
  return 0;
}

inline int getattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
  try {
    auto inode_idx = get_inode_idx(path, fi);
    auto disk_inode = nfs.get_diskinode(inode_idx);
    stbuf->st_mode = disk_inode->mode;
    stbuf->st_atime = disk_inode->access_time;
    stbuf->st_mtime = disk_inode->modify_time;
    stbuf->st_ctime = disk_inode->change_time;
    stbuf->st_size = disk_inode->size;
    stbuf->st_mode = disk_inode->mode;
    stbuf->st_nlink = disk_inode->link_cnt;
    stbuf->st_uid = disk_inode->uid;
    stbuf->st_gid = disk_inode->gid;
    stbuf->st_blocks = disk_inode->st_blocks();
    stbuf->st_blksize = kBlockSize;
  } catch (NoEntry &e) {
    return -ENOENT;
  }
  return 0;
}

} // namespace vfs