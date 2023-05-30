#pragma once

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

inline std::vector<std::string> parse_path_components(const char *) {
  // todo
  return {};
}

inline void debug(const std::string &msg) {
  #ifndef NDEBUG
    fprintf(stderr, "DEBUG >>>> %s\n", msg.c_str());
  #endif
}
