#include <iostream>

#include <fuse3/fuse.h>

#include "vfs.hpp"

struct options {
  const char *disk_path;
};

#define OPTION(t, p)                                                           \
  { t, offsetof(struct options, p), 1 }

fuse_args fuse_init(int argc, char **argv) {
  fuse_opt option_spec[] = {OPTION("--disk_path=%s", disk_path), FUSE_OPT_END};
  options opt = {
      .disk_path = "/tmp/disk",
  };
  fuse_args args = FUSE_ARGS_INIT(argc, argv);
  if (fuse_opt_parse(&args, &opt, option_spec, nullptr) == -1) {
    throw std::runtime_error("Failed to parse fuse options");
  }
  return args;
}

fuse_operations bind_ops() {
  fuse_operations nfs_op = {
      .readdir = vfs::readdir,
  };
  return nfs_op;
}

int main(int argc, char **argv) {
  auto args = fuse_init(argc, argv);
  auto nfs_op = bind_ops();
  fuse_main(args.argc, args.argv, &nfs_op, nullptr);
  return 0;
}
