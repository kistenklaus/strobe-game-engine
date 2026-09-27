#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/rm.hpp"

#include <Windows.h>

#include <array>
#include <cwchar>
#include <stdexcept>
#include <system_error>

namespace strobe::fs {
namespace {

constexpr std::size_t max_path = 32768;
using path_buffer = std::array<wchar_t, max_path>;

[[noreturn]] void throw_last_error(const char *message) {
  throw std::system_error(static_cast<int>(GetLastError()),
                          std::system_category(), message);
}

std::size_t utf8_to_utf16(PathView path, path_buffer &out) {
  const int input_size = static_cast<int>(path.size());
  if (input_size <= 0 || input_size >= static_cast<int>(out.size()))
    throw std::invalid_argument("Path must not be empty or too long");
  const int converted = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), input_size, out.data(),
      static_cast<int>(out.size() - 1));
  if (converted <= 0)
    throw_last_error("Failed to convert UTF-8 path to UTF-16");
  out[static_cast<std::size_t>(converted)] = L'\0';
  return static_cast<std::size_t>(converted);
}

void make_writable(const wchar_t *path, bool force) {
  if (!force)
    return;
  if (!SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL))
    throw_last_error("Failed to clear file attributes");
}

void remove_tree(path_buffer &path, std::size_t path_length, bool force) {
  std::size_t pattern_length = path_length;
  if (pattern_length + 2 >= path.size())
    throw std::invalid_argument("Path is too long for Windows");
  if (pattern_length != 0 && path[pattern_length - 1] != L'\\' &&
      path[pattern_length - 1] != L'/')
    path[pattern_length++] = L'\\';
  path[pattern_length++] = L'*';
  path[pattern_length] = L'\0';

  WIN32_FIND_DATAW entry{};
  HANDLE find = FindFirstFileW(path.data(), &entry);
  path[path_length] = L'\0';
  if (find == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND)
      return;
    throw_last_error("Failed to enumerate directory");
  }

  do {
    if (std::wcscmp(entry.cFileName, L".") == 0 ||
        std::wcscmp(entry.cFileName, L"..") == 0)
      continue;

    const std::size_t old_length = path_length;
    const std::size_t name_length = std::wcslen(entry.cFileName);
    const bool separator = path_length != 0 && path[path_length - 1] != L'\\' &&
                           path[path_length - 1] != L'/';
    if (path_length + (separator ? 1 : 0) + name_length >= path.size()) {
      FindClose(find);
      throw std::invalid_argument("Path is too long for Windows");
    }
    if (separator)
      path[path_length++] = L'\\';
    std::wmemcpy(path.data() + path_length, entry.cFileName, name_length);
    path_length += name_length;
    path[path_length] = L'\0';

    const bool directory =
        (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    const bool reparse_point =
        (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    if (directory && !reparse_point) {
      remove_tree(path, path_length, force);
      make_writable(path.data(), force);
      if (!RemoveDirectoryW(path.data())) {
        FindClose(find);
        throw_last_error("Failed to remove directory");
      }
    } else {
      make_writable(path.data(), force);
      if (!DeleteFileW(path.data())) {
        FindClose(find);
        throw_last_error("Failed to remove file");
      }
    }

    path_length = old_length;
    path[path_length] = L'\0';
  } while (FindNextFileW(find, &entry));

  const DWORD enumeration_error = GetLastError();
  FindClose(find);
  if (enumeration_error != ERROR_NO_MORE_FILES)
    throw std::system_error(static_cast<int>(enumeration_error),
                            std::system_category(),
                            "Failed to enumerate directory");
}

} // namespace

void rm(PathView path, const Stat *, RmFlags flags) {
  path_buffer wide_path{};
  const std::size_t path_length = utf8_to_utf16(path, wide_path);
  const bool force = (flags & RmFlags::force) != RmFlags::none;
  const bool recursive = (flags & RmFlags::recursive) != RmFlags::none;

  const DWORD attributes = GetFileAttributesW(wide_path.data());
  if (attributes == INVALID_FILE_ATTRIBUTES)
    throw_last_error("Failed to inspect path");
  const bool directory = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  const bool reparse_point = (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;

  if (directory && !reparse_point) {
    if (!recursive)
      throw std::invalid_argument("Cannot remove a directory without recursive");
    remove_tree(wide_path, path_length, force);
    make_writable(wide_path.data(), force);
    if (!RemoveDirectoryW(wide_path.data()))
      throw_last_error("Failed to remove directory");
  } else {
    make_writable(wide_path.data(), force);
    if (!DeleteFileW(wide_path.data()))
      throw_last_error("Failed to remove file");
  }
}

} // namespace strobe::fs
