// bind fuse api w/ nfs api

#pragma once

#include <cstdio>

#include "fuse3/fuse.h"

#include "nfs/config.hpp"
#include "nfs/disk_inode.hpp"
#include "nfs/nfs.hpp"

namespace vfs {

static NaiveFS nfs;

int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
            fuse_file_info *fi, fuse_readdir_flags flags) {
  printf("readdir called\n");
  return 0;
}

int getattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
  printf("getattr called\n");
  std::unique_ptr<DiskInode> disk_inode;
  if (fi == nullptr) {
    disk_inode = nfs.getattr(path);
  } else {
    disk_inode = nfs.getattr(path, fi->fh);
  }
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