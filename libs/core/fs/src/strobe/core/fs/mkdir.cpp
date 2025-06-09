#include "./mkdir.hpp"
#include "strobe/core/fs/Path.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/stat.hpp"
#include "strobe/core/fs/utility.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <format>
#include <print>
#include <sys/syslimits.h>
#include <system_error>

#if defined(_WIN32)
static_assert(false, "Not implemented for windows");
#elifdef __linux__
#include <linux/limits.h>
#define _POSIX_C_SOURCE 200809L
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#elifdef __APPLE__
#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace strobe::fs {

/// Creates a new directory and all of its parents
/// if the directory does already exists no error is thrown.
///
/// path is a null terminated string.
static void mkdir_and_parents(std::span<char> path) {
  PathView pathView{path};
  if (exists(pathView)) {
    if (!stat(pathView).isDirectory()) {
      throw std::runtime_error(std::format(
          "Cannot create directory '{}': File exists", path.data()));
    }
    return; // directory already exists.
  } else {
    const auto begin =
        path.rbegin() + 2; // skip null terminator and the last '/' char.
    const auto rit = std::find(begin, path.rend(), '/');
    if (rit == path.rend()) {
      // Matches paths like "foo/". Here we assume that CWD exists.
      if (::mkdir(path.data(), 0755) == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to create directory '{}': {}", path.data(),
                        std::strerror(errno)));
      }
    } else {
      auto it = rit.base();
      char tmp = *it;
      *it = '\0';
      auto parentPath = path.subspan(0, std::distance(path.begin(), it));
      mkdir_and_parents(parentPath);
      *it = tmp;
      // Create directory.
      if (::mkdir(path.data(), 0755) == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to create directory '{}': {}", path.data(),
                        std::strerror(errno)));
      }
    }
  }
}

void mkdir(PathView path, MkdirFlags flags) {
  const char *cpath = path.c_str();
  if (flags & MkdirFlagBits::Parents) {
#ifdef __linux__
    char pathBuffer[PATH_MAX];
    if (path.size() + 1 >= PATH_MAX) {
      throw std::runtime_error("Normalized path exceeds PATH_MAX");
    }
    std::size_t pathSize = path.size();
    std::memcpy(pathBuffer, path.c_str(), (path.size() + 1) * sizeof(char));
    if (pathBuffer[pathSize - 1] != '/') {
      pathBuffer[pathSize] = '/';
      pathSize += 1;
    }
    std::size_t normalizedSize =
        details::normalize_path_inplace(std::span<char>(pathBuffer, pathSize));
    assert(normalizedSize + 1 < PATH_MAX);
    pathBuffer[normalizedSize] = '\0';
#elifdef __APPLE__
    char pathBuffer[PATH_MAX];
    if (path.size() + 1 >= PATH_MAX) {
      throw std::runtime_error("Normalized path exceeds PATH_MAX");
    }
    std::size_t pathSize = path.size();
    std::memcpy(pathBuffer, path.c_str(), (path.size() + 1) * sizeof(char));
    if (pathBuffer[pathSize - 1] != '/') {
      pathBuffer[pathSize] = '/';
      pathSize += 1;
    }
    std::size_t normalizedSize =
        details::normalize_path_inplace(std::span<char>(pathBuffer, pathSize));
    assert(normalizedSize + 1 < PATH_MAX);
    pathBuffer[normalizedSize] = '\0';
#endif
    mkdir_and_parents(std::span<char>(pathBuffer, normalizedSize + 1));
  } else {
    if (::mkdir(cpath, 0755) == -1) {
      throw std::system_error(errno, std::generic_category(),
                              std::format("Failed to create directory '{}': {}",
                                          cpath, std::strerror(errno)));
    }
  }
}

} // namespace strobe::fs
