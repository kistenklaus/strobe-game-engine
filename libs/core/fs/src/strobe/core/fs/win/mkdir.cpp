#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/mkdir.hpp"

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
    throw std::invalid_argument("Directory path must not be empty");
  const int converted = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), input_size, out.data(),
      static_cast<int>(out.size() - 1));
  if (converted <= 0)
    throw_last_error("Failed to convert UTF-8 path to UTF-16");
  out[static_cast<std::size_t>(converted)] = L'\0';
  return static_cast<std::size_t>(converted);
}

bool is_separator(wchar_t value) noexcept {
  return value == L'/' || value == L'\\';
}

void create_one(const wchar_t *path) {
  if (CreateDirectoryW(path, nullptr))
    return;
  const DWORD error = GetLastError();
  if (error == ERROR_ALREADY_EXISTS) {
    const DWORD attributes = GetFileAttributesW(path);
    if (attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
      return;
    SetLastError(error);
  }
  throw_last_error("Failed to create directory");
}

std::size_t parent_root_length(const path_buffer &path, std::size_t length) {
  // A drive root (C:\) is already provided by the volume.
  if (length >= 3 && path[1] == L':' && is_separator(path[2]))
    return 3;

  // For a UNC path, skip the server and share components. They form the
  // network root and must not be passed to CreateDirectoryW as parents.
  if (length >= 2 && is_separator(path[0]) && is_separator(path[1])) {
    std::size_t position = 2;
    for (int component = 0; component != 2; ++component) {
      while (position < length && is_separator(path[position]))
        ++position;
      while (position < length && !is_separator(path[position]))
        ++position;
    }
    while (position < length && is_separator(path[position]))
      ++position;
    return position;
  }
  return 0;
}

void create_with_parents(path_buffer &path, std::size_t length) {
  const std::size_t root_length = parent_root_length(path, length);
  std::size_t component_start = root_length;
  for (std::size_t position = root_length; position < length; ++position) {
    if (!is_separator(path[position]))
      continue;
    if (position == component_start) {
      component_start = position + 1;
      continue;
    }
    const wchar_t saved = path[position];
    path[position] = L'\0';
    create_one(path.data());
    path[position] = saved;
    component_start = position + 1;
  }
  if (component_start < length)
    create_one(path.data());
  else if (root_length == 0 && length != 0)
    create_one(path.data());
}

} // namespace

void mkdir(PathView path, MkdirFlags flags) {
  path_buffer wide_path{};
  const std::size_t length = utf8_to_utf16(path, wide_path);
  if ((flags & MkdirFlags::parents) != MkdirFlags::none)
    create_with_parents(wide_path, length);
  else
    create_one(wide_path.data());
}

} // namespace strobe::fs
