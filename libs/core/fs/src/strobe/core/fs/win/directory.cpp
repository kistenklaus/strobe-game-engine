#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/directory.hpp"

#include <Windows.h>
#include <winioctl.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace strobe::fs {
namespace {

struct WinDirectoryState {
  HANDLE handle = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATAW data{};
  bool first = true;
  wchar_t pattern[8192]{};
};

static_assert(sizeof(WinDirectoryState) <= 32768);

bool is_separator(wchar_t value) noexcept {
  return value == L'/' || value == L'\\';
}

bool is_dot_entry(const wchar_t *name) noexcept {
  return name[0] == L'.' &&
         (name[1] == L'\0' || (name[1] == L'.' && name[2] == L'\0'));
}

[[noreturn]] void throw_last_error(const char *message) {
  throw std::system_error(static_cast<int>(GetLastError()),
                          std::system_category(), message);
}

} // namespace

Directory::Directory(PathView path) {
  auto *state_storage = reinterpret_cast<WinDirectoryState *>(
      m_platform_state.data());
  ::new (state_storage) WinDirectoryState{};
  WinDirectoryState &state =
      *state_storage;
  const int input_size = static_cast<int>(path.size());
  const int converted = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), input_size, state.pattern,
      static_cast<int>(std::size(state.pattern) - 3));
  if (converted <= 0) {
    throw_last_error("Failed to convert UTF-8 directory path");
  }

  int path_size = converted;
  if (path_size == 0 || !is_separator(state.pattern[path_size - 1])) {
    state.pattern[path_size++] = L'\\';
  }
  state.pattern[path_size] = L'\0';

  const DWORD attributes = GetFileAttributesW(state.pattern);
  if (attributes == INVALID_FILE_ATTRIBUTES) {
    throw_last_error("Failed to inspect directory");
  }
  if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    SetLastError(ERROR_DIRECTORY);
    throw_last_error("Path is not a directory");
  }

  state.pattern[path_size++] = L'*';
  state.pattern[path_size] = L'\0';
  state.handle = FindFirstFileW(state.pattern, &state.data);
  if (state.handle == INVALID_HANDLE_VALUE &&
      GetLastError() != ERROR_FILE_NOT_FOUND) {
    throw_last_error("Failed to open directory");
  }
  state.first = true;
  m_handle = state.handle == INVALID_HANDLE_VALUE
                 ? nullptr
                 : reinterpret_cast<void *>(state.handle);
  m_opened = true;
}

Directory &Directory::operator=(Directory &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  close_platform();
  m_handle = std::exchange(other.m_handle, nullptr);
  m_opened = std::exchange(other.m_opened, false);
  m_platform_state = other.m_platform_state;
  m_name = other.m_name;
  return *this;
}

Directory::~Directory() noexcept { close_platform(); }

void Directory::close_platform() noexcept {
  if (m_opened) {
    WinDirectoryState &state =
      *reinterpret_cast<WinDirectoryState *>(m_platform_state.data());
    if (state.handle != INVALID_HANDLE_VALUE) {
      FindClose(state.handle);
    }
  }
  m_handle = nullptr;
  m_opened = false;
}

void Directory::close() {
  if (!m_opened) {
    return;
  }
  WinDirectoryState &state =
      *reinterpret_cast<WinDirectoryState *>(m_platform_state.data());
  if (state.handle != INVALID_HANDLE_VALUE && !FindClose(state.handle)) {
    state.handle = INVALID_HANDLE_VALUE;
    m_handle = nullptr;
    m_opened = false;
    throw_last_error("Failed to close directory");
  }
  state.handle = INVALID_HANDLE_VALUE;
  m_handle = nullptr;
  m_opened = false;
}

bool Directory::advance(EntryView &entry) {
  if (!m_opened) {
    return false;
  }
  WinDirectoryState &state =
      *reinterpret_cast<WinDirectoryState *>(m_platform_state.data());
  while (state.handle != INVALID_HANDLE_VALUE) {
    if (!state.first && !FindNextFileW(state.handle, &state.data)) {
      if (GetLastError() == ERROR_NO_MORE_FILES) {
        return false;
      }
      throw_last_error("Failed to enumerate directory");
    }
    state.first = false;
    if (is_dot_entry(state.data.cFileName)) {
      continue;
    }

    const int converted = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, state.data.cFileName, -1, m_name.data(),
        static_cast<int>(m_name.size()), nullptr, nullptr);
    if (converted <= 0) {
      throw_last_error("Failed to convert directory entry name");
    }
    Type type = (state.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ? Type::Directory
                    : Type::File;
    if ((state.data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 &&
        state.data.dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
      type = Type::SymLink;
    }
    entry = EntryView{
        PathView(std::span<const char>(m_name.data(), converted)), type};
    return true;
  }
  return false;
}

Directory::iterator Directory::begin() {
  iterator result(this);
  result.m_end = !advance(result.m_entry);
  if (result.m_end) {
    result.m_directory = nullptr;
  }
  return result;
}

Directory::iterator &Directory::iterator::operator++() {
  if (m_end) {
    return *this;
  }
  m_end = !m_directory->advance(m_entry);
  if (m_end) {
    m_directory = nullptr;
  }
  return *this;
}

Directory ls(PathView path) { return Directory(path); }

} // namespace strobe::fs
