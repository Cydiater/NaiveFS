#include <iostream>

#include <fuse3/fuse.h>

#include "vfs.hpp"

fuse_args fuse_init(int argc, char **argv) {
  fuse_args args = FUSE_ARGS_INIT(argc, argv);
  return args;
}

fuse_operations bind_ops() {
  fuse_operations nfs_op = {
      .getattr = vfs::getattr,
      .unlink = vfs::unlink,
      .open = vfs::open,
      .read = vfs::read,
      .write = vfs::write,
      .fsync = vfs::fsync,
      .readdir = vfs::readdir,
      .access = vfs::access,
      .create = vfs::create,
      .utimens = vfs::utimens,
  };
  return nfs_op;
}

int main(int argc, char **argv) {
  auto args = fuse_init(argc, argv);
  auto nfs_op = bind_ops();
  fuse_main(args.argc, args.argv, &nfs_op, nullptr);
  return 0;
}
