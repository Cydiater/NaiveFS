// bind fuse api w/ nfs api

#pragma once

#include <cstdio>
#include <exception>

#include "fuse3/fuse.h"
#include "nfs/utils.hpp"
#include "unistd.h"

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/nfs.hpp"

namespace vfs {

static NaiveFS nfs;

inline uint32_t get_inode_idx(const char *path, fuse_file_info *fi) {
  if (fi != nullptr) {
    try {
      return nfs.get_inode_idx(fi->fh);
    } catch (const NoFd &e) {
      // continue to query by path
    }
  }
  return nfs.get_inode_idx(path);
}

int fsync(const char *path, int datasync, struct fuse_file_info *fi) {
  printf("fsync called\n");
  return 0;
}

int create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  printf("create called\n");
}

int open(const char *path, struct fuse_file_info *fi) {
  printf("open called\n");
}

int utimens(const char *path, const struct timespec tv[2],
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

int write(const char *path, const char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi) {
  printf("write called\n");
  return 0;
}

int access(const char *, int) {
  // todo: add check here
  return F_OK;
}

int read(const char *path, char *buf, size_t size, off_t offset,
         struct fuse_file_info *fi) {
  printf("read called\n");
  return 0;
}

int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
            fuse_file_info *fi, fuse_readdir_flags flags) {
  printf("readdir called\n");
  return 0;
}

int getattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
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
  return 0;
}

} // namespace vfs