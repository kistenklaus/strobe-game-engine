#include "./stat.hpp"
#include <format>
#include <sys/stat.h>

namespace strobe::fs {

Stat stat(PathView path, StatFlags flags) {
  Stat out;
  struct stat stat;
  if (flags & StatFlagBits::FollowSymLink) {
    if (::stat(path.c_str(), &stat) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to lstat '{}'", path.c_str()));
    }
  } else {
    if (::lstat(path.c_str(), &stat) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to lstat '{}'", path.c_str()));
    }
  }
  if (S_ISREG(stat.st_mode)) {
    out.m_type = Type::File;
  }
  if (S_ISDIR(stat.st_mode)) {
    out.m_type = Type::Directory;
  } else if (S_ISLNK(stat.st_mode)) {
    out.m_type = Type::SymLink;
  }
  out.m_size = stat.st_size;
  return out;
}

} // namespace strobe::fs
