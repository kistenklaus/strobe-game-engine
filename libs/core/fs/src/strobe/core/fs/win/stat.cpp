#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/stat.hpp"

#include <Windows.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <system_error>

namespace strobe::fs {
namespace {

constexpr std::size_t max_path = 32768;

[[noreturn]] void throw_last_error(const char *message) {
  throw std::system_error(static_cast<int>(GetLastError()),
                          std::system_category(), message);
}

std::size_t utf8_to_utf16(PathView path,
                          std::array<wchar_t, max_path> &out) {
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

} // namespace

Stat stat(PathView path, StatFlags flags) {
  std::array<wchar_t, max_path> wide_path{};
  utf8_to_utf16(path, wide_path);

  WIN32_FILE_ATTRIBUTE_DATA data{};
  if (!GetFileAttributesExW(wide_path.data(), GetFileExInfoStandard, &data))
    throw_last_error("Failed to stat path");

  Stat result;
  const bool reparse =
      (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
  if (reparse && (flags & StatFlags::follow_symlink) == StatFlags::none) {
    result.m_type = Type::SymLink;
  } else if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    result.m_type = Type::Directory;
  } else {
    result.m_type = Type::File;
  }
  const std::uint64_t file_size =
      (static_cast<std::uint64_t>(data.nFileSizeHigh) << 32) |
      static_cast<std::uint64_t>(data.nFileSizeLow);
  result.m_size = static_cast<std::size_t>(file_size);
  return result;
}

}
