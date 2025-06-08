#include "./mv.hpp"
#include "strobe/core/fs/rm.hpp"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <format>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

namespace strobe::fs {

void mv(PathView src, PathView dst, MvFlags flags) {
  // Stat destination
  struct stat dst_stat;
  if (::stat(dst.c_str(), &dst_stat) == 0) {
    if ((flags & MvFlagBits::Force) == MvFlagBits::Force) {
      // If destination is a directory, do NOT remove it — that’s not what mv -f
      // does!
      if (!S_ISDIR(dst_stat.st_mode)) {
        if (::unlink(dst.c_str()) != 0) {
          throw std::system_error(
              errno, std::generic_category(),
              std::format("Failed to remove destination file '{}': {}",
                          dst.c_str(), std::strerror(errno)));
        }
      }
    } else {
      throw std::runtime_error(std::format(
          "Destination '{}' exists and Force flag not set", dst.c_str()));
    }
  }

  // Stat source
  struct stat src_stat;
  if (::stat(src.c_str(), &src_stat) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            std::format("Failed to stat source '{}': {}",
                                        src.c_str(), std::strerror(errno)));
  }

  // Save timestamps if needed (must be done before rename to avoid
  // rename-induced atime changes)
  struct timeval times[2];
  bool preserveTimestamps = (flags & MvFlagBits::PreserveTimestamps) ==
                            MvFlagBits::PreserveTimestamps;
  if (preserveTimestamps) {
#ifdef __APPLE__
    times[0].tv_sec = src_stat.st_atimespec.tv_sec;
    times[0].tv_usec = src_stat.st_atimespec.tv_nsec / 1000;
    times[1].tv_sec = src_stat.st_mtimespec.tv_sec;
    times[1].tv_usec = src_stat.st_mtimespec.tv_nsec / 1000;
#else
    times[0].tv_sec = src_stat.st_atim.tv_sec;
    times[0].tv_usec = src_stat.st_atim.tv_nsec / 1000;
    times[1].tv_sec = src_stat.st_mtim.tv_sec;
    times[1].tv_usec = src_stat.st_mtim.tv_nsec / 1000;
#endif
  }

  // Perform the rename
  if (::rename(src.c_str(), dst.c_str()) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            std::format("Failed to move '{}' to '{}': {}",
                                        src.c_str(), dst.c_str(),
                                        std::strerror(errno)));
  }

  // Reapply timestamps
  if (preserveTimestamps) {
    if (::utimes(dst.c_str(), times) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to preserve timestamps on '{}': {}", dst.c_str(),
                      std::strerror(errno)));
    }
  }
}

} // namespace strobe::fs
