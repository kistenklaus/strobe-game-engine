#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/mv.hpp"

#include <Windows.h>

#include <array>
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

struct timestamps {
  FILETIME creation;
  FILETIME access;
  FILETIME write;
};

timestamps read_timestamps(const wchar_t *path) {
  HANDLE handle = CreateFileW(
      path, GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle == INVALID_HANDLE_VALUE)
    throw_last_error("Failed to open source for timestamp preservation");
  timestamps result{};
  const bool success = GetFileTime(handle, &result.creation, &result.access,
                                   &result.write) != FALSE;
  const DWORD error = success ? ERROR_SUCCESS : GetLastError();
  CloseHandle(handle);
  if (!success) {
    SetLastError(error);
    throw_last_error("Failed to read source timestamps");
  }
  return result;
}

void write_timestamps(const wchar_t *path, const timestamps &value) {
  HANDLE handle = CreateFileW(
      path, FILE_WRITE_ATTRIBUTES,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle == INVALID_HANDLE_VALUE)
    throw_last_error("Failed to open destination for timestamp preservation");
  const bool success = SetFileTime(handle, &value.creation, &value.access,
                                   &value.write) != FALSE;
  const DWORD error = success ? ERROR_SUCCESS : GetLastError();
  CloseHandle(handle);
  if (!success) {
    SetLastError(error);
    throw_last_error("Failed to preserve timestamps");
  }
}

} // namespace

void mv(PathView src, PathView dst, MvFlags flags) {
  path_buffer source{}, destination{};
  utf8_to_utf16(src, source);
  utf8_to_utf16(dst, destination);

  const DWORD destination_attributes = GetFileAttributesW(destination.data());
  const bool destination_exists =
      destination_attributes != INVALID_FILE_ATTRIBUTES;
  const bool force = (flags & MvFlags::force) != MvFlags::none;
  if (destination_exists && !force)
    throw std::runtime_error("Destination exists and force flag is not set");
  if (destination_exists && force &&
      (destination_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    if (!DeleteFileW(destination.data()))
      throw_last_error("Failed to remove destination file");
  }

  const bool preserve =
      (flags & MvFlags::preserve_timestamps) != MvFlags::none;
  const timestamps source_times = preserve ? read_timestamps(source.data())
                                           : timestamps{};
  if (!MoveFileExW(source.data(), destination.data(),
                   MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
    throw_last_error("Failed to move path");
  if (preserve)
    write_timestamps(destination.data(), source_times);
}

}
