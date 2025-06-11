#include "./rm.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/stat.hpp"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

namespace strobe::fs {

/// Takes ownership of the fd.
static void rm_dir_content_recursive(int fd) {
  DIR *dir = ::fdopendir(fd);
  if (dir == nullptr) {
    ::close(fd);
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to fdopendir: {}", std::strerror(errno)));
  }

  struct dirent *ent;
  while ((ent = ::readdir(dir)) != nullptr) {
    if ((std::strcmp(ent->d_name, ".") == 0) ||
        (std::strcmp(ent->d_name, "..") == 0)) {
      continue;
    }
    std::flush(std::cout);
    bool recurse;
    if (ent->d_type == DT_DIR) {
      recurse = true;
    } else if (ent->d_type == DT_UNKNOWN) {
      struct stat stat;
      if (::fstatat(fd, ent->d_name, &stat, AT_SYMLINK_NOFOLLOW) < 0) {
        ::closedir(dir);
        throw std::system_error(errno, std::generic_category(),
                                fmt::format("Failed to stat '{}': {}",
                                            ent->d_name, std::strerror(errno)));
        // TODO error with fstatat.
      }
      recurse = S_ISDIR(stat.st_mode);
    } else {
      recurse = false;
    }
    if (recurse) {
      int subdir = ::openat(dirfd(dir), ent->d_name, O_RDONLY | O_DIRECTORY);
      if (subdir < 0) {
        ::closedir(dir);
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open subdirectory '{}': {}", ent->d_name,
                        std::strerror(errno)));
      }
      rm_dir_content_recursive(subdir);
      if (::unlinkat(dirfd(dir), ent->d_name, AT_REMOVEDIR) != 0) {
        ::closedir(dir);
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to remove directory '{}': {}", ent->d_name,
                        std::strerror(errno)));
      }

    } else {
      if (::unlinkat(fd, ent->d_name, 0) != 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to remove file: {}", std::strerror(errno)));
      }
    }
  }
  if (::closedir(dir) != 0) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to close directory: {}", std::strerror(errno)));
  }
}

void rm(PathView path, const Stat *stat, RmFlags flags) {
  if (stat->isDirectory()) {
    if (!(flags & RmFlagBits::Recursive)) {
      throw std::invalid_argument(
          fmt::format("Cannot remove '{}': Is a directory", path.c_str()));
      return;
    }
    int fd = ::open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
      throw std::system_error(errno, std::generic_category(),
                              fmt::format("Failed to open directory '{}': {}",
                                          path.c_str(), std::strerror(errno)));
    }
    try {
      rm_dir_content_recursive(fd);
    } catch (const std::exception &e) {
      throw std::runtime_error(fmt::format(
          "Failed to remove directory '{}': \n{}", path.c_str(), e.what()));
    }
    if (::rmdir(path.c_str()) != 0) {
      throw std::system_error(errno, std::generic_category(),
                              fmt::format("Failed to remove directory '{}': {}",
                                          path.c_str(), std::strerror(errno)));
    }

  } else {
    // files or links.
    if (::unlink(path.c_str()) != 0) {
      throw std::system_error(errno, std::generic_category(),
                              fmt::format("Failed to remove file '{}': {}",
                                          path.c_str(), std::strerror(errno)));
    }
  }
}

void rm(PathView path, RmFlags flags) {
  if (!strobe::fs::exists(path)) {
    if (!(flags & RmFlagBits::Force)) {
      throw std::invalid_argument(fmt::format(
          "Cannot remove '{}': No such file or directory", path.c_str()));
    }
    return;
  }

  Stat stat = strobe::fs::stat(path);
  rm(path, &stat, flags);
}

} // namespace strobe::fs
  //
