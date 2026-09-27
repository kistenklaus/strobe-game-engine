#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "strobe/core/fs/cp.hpp"

#include <Windows.h>

#include <array>
#include <cwchar>
#include <stdexcept>
#include <system_error>

namespace strobe::fs {
namespace {

// Keep the successful copy path allocation-free. Windows extended paths are
// limited to 32,767 UTF-16 code units.
constexpr std::size_t max_path = 32768;
using path_buffer = std::array<wchar_t, max_path>;

[[noreturn]] void throw_last_error(const char *message) {
  throw std::system_error(static_cast<int>(GetLastError()),
                          std::system_category(), message);
}

std::size_t utf8_to_utf16(PathView path, path_buffer &out) {
  const int size = static_cast<int>(path.size());
  if (size >= static_cast<int>(out.size()))
    throw std::invalid_argument("Path is too long for Windows");
  const int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                            path.c_str(), size, out.data(),
                                            static_cast<int>(out.size() - 1));
  if (converted <= 0)
    throw_last_error("Failed to convert UTF-8 path to UTF-16");
  out[static_cast<std::size_t>(converted)] = L'\0';
  return static_cast<std::size_t>(converted);
}

bool is_directory(const path_buffer &path) {
  const DWORD attributes = GetFileAttributesW(path.data());
  if (attributes == INVALID_FILE_ATTRIBUTES)
    throw_last_error("Failed to inspect source path");
  return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void preserve_metadata(const wchar_t *source, const wchar_t *destination,
                       bool directory) {
  const DWORD source_attributes = GetFileAttributesW(source);
  if (source_attributes == INVALID_FILE_ATTRIBUTES)
    throw_last_error("Failed to inspect source metadata");

  HANDLE source_handle = CreateFileW(
      source, GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, directory ? FILE_FLAG_BACKUP_SEMANTICS : 0, nullptr);
  if (source_handle == INVALID_HANDLE_VALUE)
    throw_last_error("Failed to open source metadata");

  FILETIME creation{}, access{}, write{};
  const bool got_time = GetFileTime(source_handle, &creation, &access, &write);
  const DWORD time_error = got_time ? ERROR_SUCCESS : GetLastError();
  CloseHandle(source_handle);
  if (!got_time) {
    SetLastError(time_error);
    throw_last_error("Failed to read source timestamps");
  }

  HANDLE destination_handle = CreateFileW(
      destination, FILE_WRITE_ATTRIBUTES,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
      OPEN_EXISTING, directory ? FILE_FLAG_BACKUP_SEMANTICS : 0, nullptr);
  if (destination_handle == INVALID_HANDLE_VALUE)
    throw_last_error("Failed to open destination metadata");
  const bool set_time = SetFileTime(destination_handle, &creation, &access,
                                     &write) != FALSE;
  const DWORD set_error = set_time ? ERROR_SUCCESS : GetLastError();
  CloseHandle(destination_handle);
  if (!set_time) {
    SetLastError(set_error);
    throw_last_error("Failed to preserve timestamps");
  }

  if (!SetFileAttributesW(destination, source_attributes))
    throw_last_error("Failed to preserve file attributes");
}

void append_component(path_buffer &path, std::size_t &length,
                      const wchar_t *component) {
  const std::size_t component_length = std::wcslen(component);
  const bool separator = length != 0 && path[length - 1] != L'\\' &&
                         path[length - 1] != L'/';
  const std::size_t required = length + (separator ? 1 : 0) + component_length;
  if (required >= path.size())
    throw std::invalid_argument("Path is too long for Windows");
  if (separator)
    path[length++] = L'\\';
  std::wmemcpy(path.data() + length, component, component_length);
  length += component_length;
  path[length] = L'\0';
}

void copy_tree(path_buffer &source, std::size_t source_length,
               path_buffer &destination, std::size_t destination_length,
               bool preserve) {
  const DWORD destination_attributes = GetFileAttributesW(destination.data());
  if (destination_attributes == INVALID_FILE_ATTRIBUTES) {
    if (!CreateDirectoryW(destination.data(), nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
      throw_last_error("Failed to create destination directory");
  } else if ((destination_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    throw std::runtime_error("Directory to file copy mismatch");
  }

  // Use the source buffer for the search pattern and restore it before
  // descending into an entry. This avoids a large temporary stack buffer at
  // every recursion level.
  std::size_t pattern_length = source_length;
  append_component(source, pattern_length, L"*");
  WIN32_FIND_DATAW entry{};
  HANDLE find = FindFirstFileW(source.data(), &entry);
  source[source_length] = L'\0';
  if (find == INVALID_HANDLE_VALUE)
    throw_last_error("Failed to enumerate source directory");

  do {
    if (std::wcscmp(entry.cFileName, L".") == 0 ||
        std::wcscmp(entry.cFileName, L"..") == 0)
      continue;

    const std::size_t old_source_length = source_length;
    const std::size_t old_destination_length = destination_length;
    append_component(source, source_length, entry.cFileName);
    append_component(destination, destination_length, entry.cFileName);

    const bool child_directory =
        (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    if (child_directory) {
      copy_tree(source, source_length, destination, destination_length,
                preserve);
    } else {
      if (!CopyFileW(source.data(), destination.data(), FALSE)) {
        FindClose(find);
        throw_last_error("Failed to copy file");
      }
      if (preserve)
        preserve_metadata(source.data(), destination.data(), false);
    }
    source_length = old_source_length;
    destination_length = old_destination_length;
    source[source_length] = L'\0';
    destination[destination_length] = L'\0';
  } while (FindNextFileW(find, &entry));

  const DWORD enumeration_error = GetLastError();
  FindClose(find);
  if (enumeration_error != ERROR_NO_MORE_FILES)
    throw std::system_error(static_cast<int>(enumeration_error),
                            std::system_category(),
                            "Failed to enumerate source directory");
  if (preserve)
    preserve_metadata(source.data(), destination.data(), true);
}

} // namespace

void cp(PathView source_path, PathView destination_path, CpFlags flags) {
  path_buffer source{}, destination{};
  const std::size_t source_length = utf8_to_utf16(source_path, source);
  std::size_t destination_length =
      utf8_to_utf16(destination_path, destination);
  const bool preserve = (flags & CpFlags::preserve) != CpFlags::none;
  const bool recursive = (flags & CpFlags::recursive) != CpFlags::none;

  const bool source_directory = is_directory(source);
  const DWORD destination_attributes = GetFileAttributesW(destination.data());
  const bool destination_exists = destination_attributes != INVALID_FILE_ATTRIBUTES;
  const bool destination_directory =
      destination_exists &&
      (destination_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

  if (source_directory && !recursive)
    throw std::runtime_error("Source is a directory; recursive flag required");
  if (source_directory && destination_exists && !destination_directory)
    throw std::runtime_error("Directory to file copy mismatch");

  if (std::wcscmp(source.data(), destination.data()) == 0)
    throw std::runtime_error("Source and destination are identical");

  if (destination_directory) {
    const wchar_t *name = std::wcsrchr(source.data(), L'\\');
    const wchar_t *slash = std::wcsrchr(source.data(), L'/');
    if (slash != nullptr && (name == nullptr || slash > name))
      name = slash;
    append_component(destination, destination_length,
                     name == nullptr ? source.data() : name + 1);
  }

  if (source_directory) {
    copy_tree(source, source_length, destination, destination_length, preserve);
  } else {
    const DWORD final_attributes = GetFileAttributesW(destination.data());
    if (final_attributes != INVALID_FILE_ATTRIBUTES &&
        (final_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
      throw std::runtime_error("File to directory copy mismatch");
    if (!CopyFileW(source.data(), destination.data(), FALSE))
      throw_last_error("Failed to copy file");
    if (preserve)
      preserve_metadata(source.data(), destination.data(), false);
  }
}

} // namespace strobe::fs
