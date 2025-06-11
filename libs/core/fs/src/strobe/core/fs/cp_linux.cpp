#include "./cp.hpp"
#include "fmt/format.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>
#include <utime.h>

#include <linux/fs.h>

namespace strobe::fs {

static bool cp_file_fallback(int src_fd, int dst_fd) {
  constexpr size_t BufferSize = 16 * 1024;
  char buffer[BufferSize];
  ssize_t n;
  while ((n = ::read(src_fd, buffer, BufferSize)) > 0) {
    ssize_t written = 0;
    while (written < n) {
      ssize_t w = ::write(dst_fd, buffer + written, n - written);
      if (w < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to copy file : {}", std::strerror(errno)));
      }
      written += w;
    }
  }
  if (n < 0) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to read file : {}", std::strerror(errno)));
  }
  return n >= 0;
}

static void cp_file(int src_fd, int dst_fd, struct stat *src_stat,
                    bool preserve) {

  bool reflinked = false;

  if (ioctl(dst_fd, FICLONE, src_fd) == 0) {
    reflinked = true;
  } else if (errno != EOPNOTSUPP && errno != EXDEV) {
    ::close(src_fd);
    ::close(dst_fd);
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to reflink file: {}", std::strerror(errno)));
  }

  if (!reflinked) {
    size_t bytes_remaining = static_cast<size_t>(src_stat->st_size);
    static constexpr size_t chunk_size = 64 * 1024;

    while (bytes_remaining > 0) {
      ssize_t bytes_copied =
          ::copy_file_range(src_fd, nullptr, dst_fd, nullptr,
                            std::min(chunk_size, bytes_remaining), 0);
      if (bytes_copied == 0)
        break; // EOF reached

      if (bytes_copied < 0) {
        if (errno == EXDEV || errno == EINVAL || errno == ENOSYS) {
          ::lseek(src_fd, 0, SEEK_SET);
          ::lseek(dst_fd, 0, SEEK_SET);
          // Fallback to read/write
          try {
            cp_file_fallback(src_fd, dst_fd);
          } catch (...) {
            ::close(src_fd);
            ::close(dst_fd);
            throw;
          }
          break;
        }
        ::close(src_fd);
        ::close(dst_fd);
        // Unexpected error, propagate
        throw std::system_error(errno, std::generic_category(),
                                "copy_file_range failed");
      }

      bytes_remaining -= static_cast<size_t>(bytes_copied);
    }
  }

  if (preserve) {
    struct timeval times[2];
    times[0].tv_sec = src_stat->st_atim.tv_sec;
    times[0].tv_usec = src_stat->st_atim.tv_nsec / 1000;
    times[1].tv_sec = src_stat->st_mtim.tv_sec;
    times[1].tv_usec = src_stat->st_mtim.tv_nsec / 1000;

    if (::futimes(dst_fd, times) == -1) {
      ::close(src_fd);
      ::close(dst_fd);
      throw std::system_error(errno, std::generic_category(),
                              fmt::format("Failed to preserve timestamps : {}",
                                          std::strerror(errno)));
    }
  }

  ::close(src_fd);
  ::close(dst_fd);
}

static void cp_dir(int src_fd, int dst_fd, bool preserve) {
  DIR *dir = fdopendir(src_fd);

  struct dirent *dirent = nullptr;
  while ((dirent = readdir(dir)) != nullptr) {
    if ((std::strcmp(dirent->d_name, ".") == 0) ||
        (std::strcmp(dirent->d_name, "..") == 0)) {
      continue;
    }
    struct stat ent_src_stat;
    if (fstatat(src_fd, dirent->d_name, &ent_src_stat, 0) < 0) {
      closedir(dir);
      throw std::system_error(
          errno, std::generic_category(),
          fmt::format("Failed to stat : {}", std::strerror(errno)));
    }
    if (S_ISDIR(ent_src_stat.st_mode)) {
      int ent_src_fd = ::openat(src_fd, dirent->d_name, O_DIRECTORY);
      if (ent_src_fd < 0) {
        closedir(dir);
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open directory src entry directory '{}'",
                        dirent->d_name));
      }

      struct stat ent_dst_stat;
      if (::fstatat(dst_fd, dirent->d_name, &ent_dst_stat, 0) < 0) {
        // dst does not exists.
        mode_t mode = preserve ? ent_src_stat.st_mode : 0755;
        if (::mkdirat(dst_fd, dirent->d_name, mode) < 0) {
          ::close(ent_src_fd);
          closedir(dir);
          throw std::system_error(
              errno, std::generic_category(),
              fmt::format(
                  "Failed to create directory for dst entry directory '{}'",
                  dirent->d_name));
        }
        int ent_dst_fd = ::openat(dst_fd, dirent->d_name, O_DIRECTORY);
        if (ent_dst_fd < 0) {
          ::close(ent_src_fd);
          closedir(dir);
          throw std::system_error(
              errno, std::generic_category(),
              fmt::format(
                  "Failed to open directory for dst entry directory '{}'",
                  dirent->d_name));
        }
        cp_dir(ent_src_fd, ent_dst_fd, preserve);
        ::close(ent_dst_fd);
      } else {
        if (!S_ISDIR(ent_dst_stat.st_mode)) {
          ::close(ent_src_fd);
          closedir(dir);
          throw std::runtime_error(
              fmt::format("Failed to copy directory entry. Directory -> File "
                          "mismatch for entry '{}'",
                          dirent->d_name));
        } else {
          int ent_dst_fd = ::openat(dst_fd, dirent->d_name, O_DIRECTORY);
          if (ent_dst_fd < 0) {
            ::close(ent_src_fd);
            closedir(dir);
            throw std::system_error(
                errno, std::generic_category(),
                fmt::format(
                    "Failed to open directory for dst entry directory '{}'",
                    dirent->d_name));
          }
          cp_dir(ent_src_fd, ent_dst_fd, preserve);
          ::close(ent_dst_fd);
        }
      }
      ::close(ent_src_fd);

    } else {
      struct stat ent_dst_stat;
      int ent_src_fd = ::openat(src_fd, dirent->d_name, O_RDONLY);
      if (ent_src_fd < 0) {
        closedir(dir);
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open directory src entry '{}'",
                        dirent->d_name));
      }

      // TODO check if the dst exists and if it is check that it is not a
      // directory.
      if (fstatat(dst_fd, dirent->d_name, &ent_dst_stat, 0) < 0) {
        // dst entry does not exists.
        mode_t mode = preserve ? ent_src_stat.st_mode : 0644;
        int ent_dst_fd = ::openat(dst_fd, dirent->d_name,
                                  O_WRONLY | O_CREAT | O_TRUNC, mode);
        if (ent_dst_fd < 0) {
          closedir(dir);
          throw std::system_error(
              errno, std::generic_category(),
              fmt::format("Failed to directory dst entry '{}'",
                          dirent->d_name));
        }
        cp_file(ent_src_fd, ent_dst_fd, &ent_src_stat, preserve);
      } else {
        // dst entry does exist

        if (S_ISDIR(ent_src_stat.st_mode)) {
          closedir(dir);
          throw std::runtime_error(
              fmt::format("Failed to copy directory entry. File -> Directory "
                          "mismatch for entry '{}'",
                          dirent->d_name));
        } else {
          // dst entry does not exists.
          mode_t mode = preserve ? ent_src_stat.st_mode : 0644;
          int ent_dst_fd =
              ::openat(dst_fd, dirent->d_name, O_WRONLY | O_TRUNC, mode);
          if (ent_dst_fd < 0) {
            closedir(dir);
            throw std::system_error(
                errno, std::generic_category(),
                fmt::format("Failed to directory dst entry '{}'",
                            dirent->d_name));
          }
          cp_file(ent_src_fd, ent_dst_fd, &ent_src_stat, preserve);
        }
      }
    }
  }
  closedir(dir);
}

void cp(PathView src, PathView dst, CpFlags flags) {
  bool preserve = flags & CpFlagBits::Preserve;

  if (std::strcmp(src.c_str(), dst.c_str_name()) == 0) {
    throw std::runtime_error(
        fmt::format("Failed to copy : '{}' and '{}' are the same file",
                    src.c_str(), dst.c_str()));
  }

  struct stat src_stat;
  if (::stat(src.c_str(), &src_stat) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Cannot stat '{}': {} No such file or directory",
                    src.c_str(), std::strerror(errno)));
  }
  bool src_is_dir = S_ISDIR(src_stat.st_mode);

  if (src_is_dir && !(flags & CpFlagBits::Recursive)) {
    throw std::runtime_error(
        fmt::format("Failed to copy '{}' to '{}'. Missing recursive flag",
                    src.c_str(), dst.c_str()));
  }

  // Stat destination
  struct stat dst_stat;
  bool dst_exists = ::stat(dst.c_str(), &dst_stat) == 0;

  int dst_fd = -1;
  if (dst_exists) {
    if (S_ISDIR(dst_stat.st_mode)) {
      int dir_fd = ::open(dst.c_str(), O_DIRECTORY);
      if (dir_fd < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open destination directory '{}'",
                        dst.c_str()));
      }

      struct stat dst_stat;
      bool dst_exists = fstatat(dir_fd, src.c_str_name(), &dst_stat, 0) == 0;
      bool dst_is_dir =
          dst_exists ? S_ISDIR(dst_stat.st_mode) : S_ISDIR(src_stat.st_mode);

      if (src_is_dir != dst_is_dir) {
        ::close(dir_fd);
        if (src_is_dir) {
          throw std::runtime_error(fmt::format(
              "Cannot overwrite non-directory '{}/{}' with directory '{}'",
              dst.c_str(), src.c_str_name(), src.c_str()));
        } else {
          throw std::runtime_error(fmt::format(
              "Cannot overwrite directory '{}/{}' with directory '{}'",
              dst.c_str(), src.c_str_name(), src.c_str()));
        }
      }

      if (dst_exists) {
        if (dst_is_dir) {
          dst_fd = ::openat(dir_fd, src.c_str_name(), O_DIRECTORY);
          if (dst_fd < 0) {
            ::close(dir_fd);
            throw std::system_error(
                errno, std::generic_category(),
                fmt::format("Failed to open destination directory '{}/{}'",
                            dst.c_str(), src.c_str_name()));
          }
        } else {
          dst_fd = ::openat(dir_fd, src.c_str_name(), O_WRONLY | O_TRUNC);
          if (dst_fd < 0) {
            ::close(dir_fd);
            throw std::system_error(errno, std::generic_category(),
                                    fmt::format("Failed to open dest '{}/{}'",
                                                dst.c_str(), src.c_str_name()));
          }
        }
      } else {
        if (src_is_dir) {
          mode_t mode = preserve ? src_stat.st_mode : 0755;
          if (::mkdirat(dir_fd, src.c_str_name(), mode) < 0) {
            throw std::system_error(
                errno, std::generic_category(),
                fmt::format("Failed to create directory '{}/{}'", dst.c_str(),
                            src.c_str_name()));
          }
          dst_fd = ::openat(dir_fd, src.c_str_name(), O_DIRECTORY);
          if (dst_fd < 0) {
            ::close(dir_fd);
            throw std::system_error(
                errno, std::generic_category(),
                fmt::format("Failed to open directory '{}/{}'", dst.c_str(),
                            src.c_str_name()));
          }

        } else {
          mode_t mode = preserve ? src_stat.st_mode : 0644;
          dst_fd = ::openat(dir_fd, src.c_str_name(),
                            O_WRONLY | O_CREAT | O_TRUNC, mode);
          if (dst_fd < 0) {
            ::close(dir_fd);
            throw std::system_error(errno, std::generic_category(),
                                    fmt::format("Failed to create dest '{}/{}'",
                                                dst.c_str(), src.c_str_name()));
          }
        }
      }
      if (::close(dir_fd) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to close directory '{}'", dst.c_str()));
      }
    } else {
      // destination exists and is a file. (simply truncate)
      dst_fd = ::open(dst.c_str(), O_WRONLY | O_TRUNC);
      if (dst_fd < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open destination '{}'", dst.c_str()));
      }
    }
  } else {
    // destination does not exist.

    if (src_is_dir) {
      mode_t mode = preserve ? src_stat.st_mode : 0755;
      // TODO: account for preserve.
      if (::mkdir(dst.c_str(), mode) == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to create directory '{}': {}", dst.c_str(),
                        std::strerror(errno)));
      }
      dst_fd = ::open(dst.c_str(), O_DIRECTORY);
      if (dst_fd < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to open destination directory '{}'",
                        dst.c_str()));
      }
    } else {

      mode_t mode = preserve ? src_stat.st_mode : 0644;
      dst_fd = ::open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
      if (dst_fd < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            fmt::format("Failed to dst open file '{}'", dst.c_str()));
      }
    }
  }

  assert(dst_fd >= 0);

  int src_fd = ::open(src.c_str(), O_RDONLY);
  if (src_fd < 0) {
    ::close(dst_fd);
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to src open file '{}'", src.c_str()));
  }
  assert(src_fd >= 0);
  assert(dst_fd >= 0);

  if (src_is_dir) {
    cp_dir(src_fd, dst_fd, preserve);
  } else {
    cp_file(src_fd, dst_fd, &src_stat, preserve);
  }
}

} // namespace strobe::fs
