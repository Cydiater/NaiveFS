#pragma once

#include <cassert>
#include <cstring>
#include <exception>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class NoEntry : public std::exception {
public:
  const char *what() { return "No entry"; }
};

class NoImapEntry : public std::exception {
public:
  const char *what() { return "No imap entry"; }
};

class NoFd : public std::exception {
public:
  const char *what() { return "No file descriptor"; }
};

class SegmentOverflow : public std::exception {
  public:
  const char *what() { return "Segment overflow";}
};

inline std::pair<char *, uint32_t>
make_one_dir_entry(const std::string &name, const uint32_t inode_idx) {
  uint32_t len = name.length();
  auto buf = new char[4 + len + 4];
  std::memcpy(buf, &len, 4);
  std::memcpy(buf + 4, name.c_str(), len);
  std::memcpy(buf + 4 + len, &inode_idx, 4);
  *reinterpret_cast<bool *>(buf + 4 + len + 4) = false;
  return {buf, 4 + len + 4 + 1};
}

inline std::tuple<std::string, uint32_t, bool>
parse_one_dir_entry(const char *buf, uint32_t &offset) {
  uint32_t len = 0, inode_idx = 0;
  std::memcpy(&len, buf + offset, 4);
  assert(len < 256);
  assert(len > 0);
  offset += 4;
  std::string name(buf + offset, len);
  offset += len;
  std::memcpy(&inode_idx, buf + offset, 4);
  offset += 4;
  bool deleted = false;
  std::memcpy(&deleted, buf + offset, 1);
  offset += 1;
  return {name, inode_idx, deleted};
}

inline std::string
join_path_components(const std::vector<std::string> &path_components) {
  std::string res = "/";
  for (auto &name : path_components)
    res += name + "/";
  return res;
}

inline std::vector<std::string> parse_path_components(const char *path) {
  auto len = std::strlen(path);
  std::vector<std::string> components;
  uint32_t last = 0;
  assert(path[0] == '/');
  for (uint32_t i = 1; i < len; i++) {
    if (path[i] == '/') {
      std::string component = std::string(path + last + 1, path + i);
      components.push_back(component);
      last = i;
    }
  }
  if (path[len - 1] != '/') {
    components.push_back(std::string(path + last + 1, path + len));
  }
  return components;
}

inline void debug(const std::string &msg) {
#ifndef NDEBUG
  fprintf(stderr, "\033[1mDEBUG >>>> %s\033[0m\n", msg.c_str());
#endif
}
