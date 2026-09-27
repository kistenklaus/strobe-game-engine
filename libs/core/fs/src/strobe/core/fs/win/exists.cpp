#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/exists.hpp"

#include <Windows.h>

#include <array>

namespace strobe::fs {

bool exists(PathView path) {
  // Use a fixed stack buffer so querying a path does not allocate. Windows'
  // extended path form accepts at most 32,767 UTF-16 code units.
  constexpr std::size_t max_path = 32768;
  std::array<wchar_t, max_path> wide_path{};
  const int input_size = static_cast<int>(path.size());
  if (input_size >= static_cast<int>(wide_path.size()))
    return false;
  const int converted = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, path.c_str(), input_size,
      wide_path.data(), static_cast<int>(wide_path.size() - 1));
  if (converted <= 0)
    return false;
  wide_path[static_cast<std::size_t>(converted)] = L'\0';
  return GetFileAttributesW(wide_path.data()) != INVALID_FILE_ATTRIBUTES;
}

}
