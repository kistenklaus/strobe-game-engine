#pragma once

#include "FileAccessFlags.hpp"
#include "FileSeekFlags.hpp"
#include "Path.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
static_assert(false, "WINDOWS File implementation missing");
#else
#include <fcntl.h>    // open()
#include <sys/stat.h> // for fstat
#include <unistd.h>   // close()
#endif

namespace strobe {

class File {
public:
  File() : m_fd(-1) {}

  File(PathView path, FileAccessFlags access = FileAccessBits::Read) {
    assert(path.isFile());
    const char *cpath = path.c_str();

    int flags = 0;

    if (access & FileAccessBits::ReadWrite) {
      flags |= O_RDWR;
    } else if (access & FileAccessBits::Read) {
      flags |= O_RDONLY;
    } else if (access & FileAccessBits::Write) {
      flags |= O_WRONLY;
    } else {
      throw std::invalid_argument(
          std::format("Failed to open file '{}'. Must specify either "
                      "Read and or Write access flags.",
                      path.c_str()));
    }

    if (access & FileAccessBits::Create) {
      flags |= O_CREAT;
      if (access & FileAccessBits::Exclusive) {
        flags |= O_EXCL;
      }
    }
    if (access & FileAccessBits::Trunc) {
      flags |= O_TRUNC;
      if (!(access & FileAccessBits::Write)) {
        throw std::invalid_argument(
            std::format("Failed to open file '{}'. Invalid file access. Trunc "
                        "requires Write.",
                        path.c_str()));
      }
    }
    if (access & FileAccessBits::Append) {
      flags |= O_APPEND;
      if (!(access & FileAccessBits::Write)) {
        throw std::invalid_argument(
            std::format("Failed to open file '{}'. Invalid file access. Append "
                        "requires Write.",
                        path.c_str()));
      }
    }
    if (access & FileAccessBits::Sync) {
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
          std::format("Failed to open file '{}' (errno: {}): {}", path.c_str(),
                      errno, std::strerror(errno)));
    }
  }

  ~File() noexcept {
    if (m_fd != -1) {
      try {
        close();
      } catch (const std::system_error &e) {
        std::cerr << e.what() << std::endl;
        std::flush(std::cerr);
        std::terminate();
      }
    }
  }

  File(const File &o) = delete;
  File &operator=(const File &o) = delete;
  File(File &&o) : m_fd(std::exchange(o.m_fd, -1)) {}
  File &operator=(File &&o) {
    if (this == &o) {
      return *this;
    }
    m_fd = std::exchange(o.m_fd, -1);
    return *this;
  }

  void close() {
    if (isOpen()) {
      if (::close(std::exchange(m_fd, -1)) == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to close file (errno: {}): {}", errno,
                        std::strerror(errno)));
      }
    }
  }

  std::size_t size() const {
    assert(isOpen());

    struct stat st;
    if (::fstat(m_fd, &st) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to get file size (errno: {}): {}", errno,
                      std::strerror(errno)));
    }

    return static_cast<std::size_t>(st.st_size);
  }

  std::size_t read(std::span<std::byte> buffer) const {
    assert(isOpen());

    std::size_t total = 0;
    while (total < buffer.size()) {
      ssize_t result =
          ::read(m_fd, buffer.data() + total, buffer.size() - total);
      if (result == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to read from file (errno: {}): {}", errno,
                        std::strerror(errno)));
      }
      if (result == 0) {
        break; // EOF
      }
      total += result;
    }
    return total;
  }

  std::size_t write(std::span<const std::byte> buffer) {
    assert(isOpen());

    std::size_t total = 0;
    while (total < buffer.size()) {
      ssize_t result =
          ::write(m_fd, buffer.data() + total, buffer.size() - total);
      if (result == -1) {
        throw std::system_error(
            errno, std::generic_category(),
            std::format("Failed to write to file (errno: {}): {}", errno,
                        std::strerror(errno)));
      }
      total += result;
    }
    return total;
  }

  void seek(long offset, FileSeekFlags flags = FileSeekFlags::Set) {
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

    if (::lseek(m_fd, static_cast<off_t>(offset), os_whence) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to seek in file (errno: {}): {}", errno,
                      std::strerror(errno)));
    }
  }

  std::size_t tell() const {
    assert(isOpen());

    off_t pos = ::lseek(m_fd, 0, SEEK_CUR);
    if (pos == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to get file position (errno: {}): {}", errno,
                      std::strerror(errno)));
    }

    return static_cast<std::size_t>(pos);
  }

  void truncate(std::size_t new_size) {
    if (m_fd == -1) {
      throw std::runtime_error("File not open");
    }
    if (::ftruncate(m_fd, static_cast<off_t>(new_size)) == -1) {
      throw std::system_error(
          errno, std::generic_category(),
          std::format("Failed to truncate file (errno: {}): {}", errno,
                      std::strerror(errno)));
    }
  }

  inline bool isOpen() const { return m_fd != -1; }

  inline operator bool() const { return isOpen(); }

private:
  int m_fd;
};

} // namespace strobe
