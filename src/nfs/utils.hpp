#pragma once

#include <cassert>
#include <cstring>
#include <exception>
#include <string>
#include <utility>
#include <vector>

class NoEntry : public std::exception {
public:
  const char *what() { return "No entry"; }
};

class NoFd : public std::exception {
public:
  const char *what() { return "No file descriptor"; }
};

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
  fprintf(stderr, "DEBUG >>>> %s\n", msg.c_str());
#endif
}
