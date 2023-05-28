// bind fuse api w/ nfs api

#pragma once

namespace vfs {
int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
            fuse_file_info *fi, fuse_readdir_flags flags) {
  // todo
  return 0;
}
} // namespace vfs