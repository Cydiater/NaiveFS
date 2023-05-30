// bind fuse api w/ nfs api

#pragma once

#include "nfs/nfs.hpp"
#include <cstdio>

namespace vfs {

int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
            fuse_file_info *fi, fuse_readdir_flags flags) {
  printf("readdir called\n");
  return 0;
}

int getattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
  printf("getattr called\n");
  return 0;
}

} // namespace vfs