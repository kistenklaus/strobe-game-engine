#pragma once

#include "strobe/core/fs/Path.hpp"
#include "strobe/core/fs/stat.hpp"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <linux/limits.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace strobe {

class Directory {
public:
  Directory() : m_dir(nullptr) {}

  explicit Directory(PathView path) {
    m_dir = ::opendir(path.c_str());
    if (m_dir == nullptr) {
      throw std::system_error(errno, std::generic_category(),
                              std::format("Failed to open directory '{}': {}",
                                          path.c_str(), std::strerror(errno)));
    }
  }

  // Non-copyable
  Directory(const Directory &) = delete;
  Directory &operator=(const Directory &) = delete;

  // Movable
  Directory(Directory &&o) noexcept : m_dir(std::exchange(o.m_dir, nullptr)) {}

  Directory &operator=(Directory &&o) noexcept {
    if (this == &o) {
      return *this;
    }
    close();
    m_dir = std::exchange(o.m_dir, nullptr);
    return *this;
  }

  ~Directory() noexcept {
    try {
      close();
    } catch (const std::system_error &e) {
      std::cerr << e.what() << std::endl;
      std::terminate();
    }
  }

  struct EntryView {
    const char *name;
    fs::Type type;
  };

  std::optional<EntryView> next() {
    assert(valid());
    struct dirent *ent = ::readdir(m_dir);
    if (ent == nullptr) {
      return std::nullopt;
    }
    std::optional<EntryView> entry = EntryView{};
    if (ent->d_type == DT_REG) {
      entry->type = fs::Type::File;
    } else if (ent->d_type == DT_DIR) {
      entry->type = fs::Type::Directory;
    } else if (ent->d_type == DT_LNK) {
      entry->type = fs::Type::SymLink;
    } else if (ent->d_type == DT_UNKNOWN) {
      // fallback
      struct stat stat;
      int fd = ::dirfd(m_dir);
      assert(fd != -1);
      if (::fstatat(fd, ent->d_name, &stat, AT_SYMLINK_NOFOLLOW) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Unknown file type. Falling back to fstatat failed: {}",
                        std::strerror(errno)));
      }
      if (S_ISREG(stat.st_mode)) {
        entry->type = fs::Type::File;
      } else if (S_ISDIR(stat.st_mode)) {
        entry->type = fs::Type::Directory;
      } else if (S_ISLNK(stat.st_mode)) {
        entry->type = fs::Type::SymLink;
      } else {
        throw std::runtime_error("File type not supported");
      }
    } else {
      throw std::runtime_error("File type not supported");
    }
    entry->name = ent->d_name;
    return entry;
  }

  void rewind() {
    if (valid()) {
      ::rewinddir(m_dir);
    }
  }

  void close() {
    if (valid()) {
      if (::closedir(m_dir) != 0) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to closedir: {}", std::strerror(errno)));
      }
      m_dir = nullptr;
    }
  }

  bool valid() const { return m_dir != nullptr; }

  operator bool() const { return valid(); }

private:
  DIR *m_dir = nullptr;
};
} // namespace strobe
