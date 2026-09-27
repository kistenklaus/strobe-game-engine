#include "strobe/core/fs/file.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace strobe::fs {

File::File(PathView path, FileAccess access) {
  const char *cpath = path.c_str();
  int flags = 0;

  if ((access & FileAccess::read_write) != FileAccess::none) {
    flags |= O_RDWR;
  } else if ((access & FileAccess::read) != FileAccess::none) {
    flags |= O_RDONLY;
  } else if ((access & FileAccess::write) != FileAccess::none) {
    flags |= O_WRONLY;
  } else {
    throw std::invalid_argument(
        fmt::format("Failed to open file '{}'. Must specify either "
                    "Read and or Write access flags.",
                    path.c_str()));
  }

  if ((access & FileAccess::create) != FileAccess::none) {
    flags |= O_CREAT;
    if ((access & FileAccess::exclusive) != FileAccess::none) {
      flags |= O_EXCL;
    }
  }
  if ((access & FileAccess::trunc) != FileAccess::none) {
    flags |= O_TRUNC;
    if ((access & FileAccess::write) == FileAccess::none) {
      throw std::invalid_argument(
          fmt::format("Failed to open file '{}'. Invalid file access. Trunc "
                      "requires Write.",
                      path.c_str()));
    }
  }
  if ((access & FileAccess::append) != FileAccess::none) {
    flags |= O_APPEND;
    if ((access & FileAccess::write) == FileAccess::none) {
      throw std::invalid_argument(
          fmt::format("Failed to open file '{}'. Invalid file access. Append "
                      "requires Write.",
                      path.c_str()));
    }
  }
  if ((access & FileAccess::sync) != FileAccess::none) {
    flags |= O_SYNC;
  }

  constexpr mode_t default_mode = 0644;
  if (flags & O_CREAT) {
    m_fd = ::open(cpath, flags, default_mode);
  } else {
    m_fd = ::open(cpath, flags);
  }

  if (m_fd == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to open file '{}' (errno: {}): {}", path.c_str(),
                    errno, std::strerror(errno)));
  }
}

File::~File() noexcept {
  if (m_fd != -1) {
    try {
      close();
    } catch (const std::system_error &error) {
      std::cerr << error.what() << std::endl;
      std::flush(std::cerr);
      std::terminate();
    }
  }
}

File::File(File &&other) noexcept : m_fd(std::exchange(other.m_fd, -1)) {}

File &File::operator=(File &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  m_fd = std::exchange(other.m_fd, -1);
  return *this;
}

void File::close() {
  if (isOpen() && ::close(std::exchange(m_fd, -1)) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to close file (errno: {}): {}", errno,
                    std::strerror(errno)));
  }
}

std::size_t File::size() const {
  assert(isOpen());
  struct stat st;
  if (::fstat(static_cast<int>(m_fd), &st) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to get file size (errno: {}): {}", errno,
                    std::strerror(errno)));
  }
  return static_cast<std::size_t>(st.st_size);
}

std::size_t File::read(std::span<std::byte> buffer) const {
  assert(isOpen());
  std::size_t total = 0;
  while (total < buffer.size()) {
    const ssize_t result = ::read(static_cast<int>(m_fd), buffer.data() + total,
                                  buffer.size() - total);
    if (result == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          fmt::format("Failed to read from file (errno: {}): {}", errno,
                      std::strerror(errno)));
    }
    if (result == 0) {
      break;
    }
    total += static_cast<std::size_t>(result);
  }
  return total;
}

std::size_t File::write(std::span<const std::byte> buffer) {
  assert(isOpen());
  std::size_t total = 0;
  while (total < buffer.size()) {
    const ssize_t result =
        ::write(static_cast<int>(m_fd), buffer.data() + total,
                buffer.size() - total);
    if (result == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          fmt::format("Failed to write to file (errno: {}): {}", errno,
                      std::strerror(errno)));
    }
    total += static_cast<std::size_t>(result);
  }
  return total;
}

void File::seek(long offset, FileSeekFlags flags) {
  assert(isOpen());
  int os_whence;
  switch (flags) {
  case FileSeekFlags::Set:
    os_whence = SEEK_SET;
    break;
  case FileSeekFlags::Cur:
    os_whence = SEEK_CUR;
    break;
  case FileSeekFlags::End:
    os_whence = SEEK_END;
    break;
  default:
    throw std::invalid_argument("Invalid seek flag");
  }
  if (::lseek(static_cast<int>(m_fd), static_cast<off_t>(offset), os_whence) ==
      -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to seek in file (errno: {}): {}", errno,
                    std::strerror(errno)));
  }
}

std::size_t File::tell() const {
  assert(isOpen());
  const off_t pos = ::lseek(static_cast<int>(m_fd), 0, SEEK_CUR);
  if (pos == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to get file position (errno: {}): {}", errno,
                    std::strerror(errno)));
  }
  return static_cast<std::size_t>(pos);
}

void File::truncate(std::size_t new_size) {
  if (!isOpen()) {
    throw std::runtime_error("File not open");
  }
  if (::ftruncate(static_cast<int>(m_fd), static_cast<off_t>(new_size)) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        fmt::format("Failed to truncate file (errno: {}): {}", errno,
                    std::strerror(errno)));
  }
}

} // namespace strobe::fs
