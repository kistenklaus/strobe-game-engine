#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/file.hpp"

#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace strobe::fs {
namespace {

HANDLE handle_of(std::intptr_t value) noexcept {
  return reinterpret_cast<HANDLE>(value);
}

std::intptr_t value_of(HANDLE handle) noexcept {
  return reinterpret_cast<std::intptr_t>(handle);
}

std::wstring utf8_to_utf16(PathView path) {
  const int input_size = static_cast<int>(path.size());
  const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                           path.c_str(), input_size, nullptr, 0);
  if (required <= 0) {
    throw std::system_error(static_cast<int>(GetLastError()),
                            std::system_category(),
                            "Failed to convert UTF-8 path to UTF-16");
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(),
                          input_size, result.data(), required) <= 0) {
    throw std::system_error(static_cast<int>(GetLastError()),
                            std::system_category(),
                            "Failed to convert UTF-8 path to UTF-16");
  }
  return result;
}

[[noreturn]] void throw_last_error(const char *message) {
  throw std::system_error(static_cast<int>(GetLastError()),
                          std::system_category(), message);
}

} // namespace

File::File(PathView path, FileAccess access) {
  DWORD desired_access = 0;
  if ((access & FileAccess::read_write) != FileAccess::none) {
    desired_access = GENERIC_READ | GENERIC_WRITE;
  } else if ((access & FileAccess::read) != FileAccess::none) {
    desired_access = GENERIC_READ;
  } else if ((access & FileAccess::write) != FileAccess::none) {
    desired_access = GENERIC_WRITE;
  } else {
    throw std::invalid_argument("File access requires read or write access");
  }

  if ((access & FileAccess::append) != FileAccess::none) {
    desired_access &= ~GENERIC_WRITE;
    desired_access |= FILE_APPEND_DATA;
  }

  DWORD disposition = OPEN_EXISTING;
  if ((access & FileAccess::create) != FileAccess::none) {
    disposition = (access & FileAccess::exclusive) != FileAccess::none
                     ? CREATE_NEW
                     : ((access & FileAccess::trunc) != FileAccess::none
                            ? CREATE_ALWAYS
                            : OPEN_ALWAYS);
  } else if ((access & FileAccess::trunc) != FileAccess::none) {
    if ((access & FileAccess::write) == FileAccess::none) {
      throw std::invalid_argument("File truncation requires write access");
    }
    disposition = TRUNCATE_EXISTING;
  }

  DWORD attributes = FILE_ATTRIBUTE_NORMAL;
  if ((access & FileAccess::sync) != FileAccess::none) {
    attributes |= FILE_FLAG_WRITE_THROUGH;
  }

  const std::wstring wide_path = utf8_to_utf16(path);
  HANDLE handle = CreateFileW(
      wide_path.c_str(), desired_access,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      disposition, attributes, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    throw_last_error("Failed to open file");
  }
  m_fd = value_of(handle);
}

File::~File() noexcept {
  if (m_fd != -1) {
    CloseHandle(handle_of(m_fd));
  }
}

File::File(File &&other) noexcept
    : m_fd(std::exchange(other.m_fd, -1)) {}

File &File::operator=(File &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (m_fd != -1) {
    CloseHandle(handle_of(m_fd));
  }
  m_fd = std::exchange(other.m_fd, -1);
  return *this;
}

void File::close() {
  if (m_fd == -1) {
    return;
  }
  HANDLE handle = handle_of(std::exchange(m_fd, -1));
  if (!CloseHandle(handle)) {
    throw_last_error("Failed to close file");
  }
}

std::size_t File::size() const {
  assert(isOpen());
  LARGE_INTEGER value{};
  if (!GetFileSizeEx(handle_of(m_fd), &value)) {
    throw_last_error("Failed to get file size");
  }
  return static_cast<std::size_t>(value.QuadPart);
}

std::size_t File::read(std::span<std::byte> buffer) const {
  assert(isOpen());
  std::size_t total = 0;
  while (total < buffer.size()) {
    const DWORD chunk = static_cast<DWORD>(std::min<std::size_t>(
        buffer.size() - total, std::numeric_limits<DWORD>::max()));
    DWORD transferred = 0;
    if (!ReadFile(handle_of(m_fd), buffer.data() + total, chunk, &transferred,
                   nullptr)) {
      throw_last_error("Failed to read from file");
    }
    total += transferred;
    if (transferred == 0) {
      break;
    }
  }
  return total;
}

std::size_t File::write(std::span<const std::byte> buffer) {
  assert(isOpen());
  std::size_t total = 0;
  while (total < buffer.size()) {
    const DWORD chunk = static_cast<DWORD>(std::min<std::size_t>(
        buffer.size() - total, std::numeric_limits<DWORD>::max()));
    DWORD transferred = 0;
    if (!WriteFile(handle_of(m_fd), buffer.data() + total, chunk, &transferred,
                   nullptr)) {
      throw_last_error("Failed to write to file");
    }
    total += transferred;
    if (transferred == 0) {
      throw std::system_error(ERROR_WRITE_FAULT, std::system_category(),
                              "File write made no progress");
    }
  }
  return total;
}

void File::seek(long offset, FileSeekFlags flags) {
  assert(isOpen());
  DWORD method = FILE_BEGIN;
  switch (flags) {
  case FileSeekFlags::Set:
    method = FILE_BEGIN;
    break;
  case FileSeekFlags::Cur:
    method = FILE_CURRENT;
    break;
  case FileSeekFlags::End:
    method = FILE_END;
    break;
  default:
    throw std::invalid_argument("Invalid seek flag");
  }

  LARGE_INTEGER distance{};
  distance.QuadPart = static_cast<LONGLONG>(offset);
  if (!SetFilePointerEx(handle_of(m_fd), distance, nullptr, method)) {
    throw_last_error("Failed to seek in file");
  }
}

std::size_t File::tell() const {
  assert(isOpen());
  LARGE_INTEGER zero{};
  LARGE_INTEGER position{};
  if (!SetFilePointerEx(handle_of(m_fd), zero, &position, FILE_CURRENT)) {
    throw_last_error("Failed to get file position");
  }
  return static_cast<std::size_t>(position.QuadPart);
}

void File::truncate(std::size_t new_size) {
  assert(isOpen());
  LARGE_INTEGER current{};
  LARGE_INTEGER zero{};
  if (!SetFilePointerEx(handle_of(m_fd), zero, &current, FILE_CURRENT)) {
    throw_last_error("Failed to get file position");
  }

  LARGE_INTEGER target{};
  target.QuadPart = static_cast<LONGLONG>(new_size);
  if (!SetFilePointerEx(handle_of(m_fd), target, nullptr, FILE_BEGIN) ||
      !SetEndOfFile(handle_of(m_fd))) {
    throw_last_error("Failed to truncate file");
  }
  if (!SetFilePointerEx(handle_of(m_fd), current, nullptr, FILE_BEGIN)) {
    throw_last_error("Failed to restore file position");
  }
}

} // namespace strobe::fs
