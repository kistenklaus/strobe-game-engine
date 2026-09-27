#include "strobe/core/fs/detail/utility.hpp"
#include <cassert>
#include <cstring>
#include <iterator>

namespace strobe::fs::details {

bool is_path_separator(char value) noexcept {
#ifdef _WIN32
  return value == '/' || value == '\\';
#else
  return value == '/';
#endif
}

bool has_trailing_separator(std::span<const char> path) noexcept {
  if (path.empty()) {
    return false;
  }

  std::size_t size = path.size();
  if (path.back() == '\0') {
    if (size < 2) {
      return false;
    }
    --size;
  }
  return is_path_separator(path[size - 1]);
}

std::string_view path_name(std::span<const char> path) noexcept {
  if (path.empty()) {
    return {};
  }

  std::size_t size = path.size();
  if (path.back() == '\0') {
    --size;
  }
  while (size != 0 && is_path_separator(path[size - 1])) {
    --size;
  }

  const std::size_t end = size;
  while (size != 0 && !is_path_separator(path[size - 1])) {
    --size;
  }
  return {path.data() + size, end - size};
}

std::string_view path_extension(std::span<const char> path) noexcept {
  const std::string_view name = path_name(path);
  const std::size_t dot = name.rfind('.');
  if (dot == std::string_view::npos || dot == 0) {
    return {};
  }
  return name.substr(dot + 1);
}

std::size_t normalize_path_inplace(std::span<char> nonNullTerminatedPath) {
  assert(!nonNullTerminatedPath.empty());

  const std::size_t input_size = nonNullTerminatedPath.size();
  const bool trailing_separator = is_path_separator(nonNullTerminatedPath.back());

  bool absolute = false;
  const bool drive_prefix =
      input_size >= 2 && nonNullTerminatedPath[1] == ':' &&
      ((nonNullTerminatedPath[0] >= 'A' && nonNullTerminatedPath[0] <= 'Z') ||
       (nonNullTerminatedPath[0] >= 'a' && nonNullTerminatedPath[0] <= 'z'));
  std::size_t root_size = 0;
  std::size_t read = 0;
  std::size_t write = 0;

  if (drive_prefix) {
    if (input_size >= 3 && is_path_separator(nonNullTerminatedPath[2])) {
      absolute = true;
      nonNullTerminatedPath[2] = '/';
      root_size = 3;
      read = 3;
      write = root_size;
    } else {
      // C:foo is relative to the current directory on drive C.
      root_size = 2;
      read = 2;
      write = root_size;
    }
  } else if (input_size >= 2 && is_path_separator(nonNullTerminatedPath[0]) &&
             is_path_separator(nonNullTerminatedPath[1])) {
    // Preserve a UNC server/share root.  The two components are parsed before
    // any compaction so the operation remains safe in the same buffer.
    std::size_t server_begin = 2;
    while (server_begin < input_size &&
           is_path_separator(nonNullTerminatedPath[server_begin])) {
      ++server_begin;
    }
    std::size_t server_end = server_begin;
    while (server_end < input_size &&
           !is_path_separator(nonNullTerminatedPath[server_end])) {
      ++server_end;
    }
    std::size_t share_begin = server_end;
    while (share_begin < input_size &&
           is_path_separator(nonNullTerminatedPath[share_begin])) {
      ++share_begin;
    }
    std::size_t share_end = share_begin;
    while (share_end < input_size &&
           !is_path_separator(nonNullTerminatedPath[share_end])) {
      ++share_end;
    }

    if (server_begin < server_end && share_begin < share_end) {
      const std::size_t server_size = server_end - server_begin;
      const std::size_t share_size = share_end - share_begin;
      const std::size_t server_shift = server_begin - 2;
      std::memmove(nonNullTerminatedPath.data() + 2,
                   nonNullTerminatedPath.data() + server_begin, server_size);
      share_begin -= server_shift;
      std::memmove(nonNullTerminatedPath.data() + 3 + server_size,
                   nonNullTerminatedPath.data() + share_begin, share_size);
      nonNullTerminatedPath[2 + server_size] = '/';

      root_size = 3 + server_size + share_size;
      absolute = true;
      if (share_end < input_size) {
        nonNullTerminatedPath[root_size++] = '/';
      }
      read = share_end;
      while (read < input_size && is_path_separator(nonNullTerminatedPath[read])) {
        ++read;
      }
      write = root_size;
    }
  }

  if (root_size == 0) {
    absolute = is_path_separator(nonNullTerminatedPath.front());
    root_size = absolute ? 1 : 0;
    read = absolute ? 1 : 0;
    write = root_size;
  }

  while (read < input_size) {
    while (read < input_size && is_path_separator(nonNullTerminatedPath[read])) {
      ++read;
    }
    if (read == input_size) {
      break;
    }

    const std::size_t segment_begin = read;
    while (read < input_size && !is_path_separator(nonNullTerminatedPath[read])) {
      assert(nonNullTerminatedPath[read] != '\0');
      ++read;
    }
    const std::size_t segment_size = read - segment_begin;

    if (segment_size == 1 && nonNullTerminatedPath[segment_begin] == '.') {
      continue;
    }

    if (segment_size == 2 && nonNullTerminatedPath[segment_begin] == '.' &&
        nonNullTerminatedPath[segment_begin + 1] == '.') {
      bool removed_previous = false;
      if (write > root_size) {
        std::size_t previous_begin = write;
        while (previous_begin > root_size &&
               !is_path_separator(nonNullTerminatedPath[previous_begin - 1])) {
          --previous_begin;
        }

        const std::size_t previous_size = write - previous_begin;
        const bool previous_is_parent =
            previous_size == 2 && nonNullTerminatedPath[previous_begin] == '.' &&
            nonNullTerminatedPath[previous_begin + 1] == '.';
        if (!previous_is_parent) {
          write = previous_begin;
          if (write > root_size &&
              is_path_separator(nonNullTerminatedPath[write - 1])) {
            --write;
          }
          removed_previous = true;
        }
      }

      if (!removed_previous && !absolute) {
        if (write > root_size) {
          nonNullTerminatedPath[write++] = '/';
        }
        nonNullTerminatedPath[write++] = '.';
        nonNullTerminatedPath[write++] = '.';
      }
      continue;
    }

    if (write > root_size) {
      nonNullTerminatedPath[write++] = '/';
    }
    std::memmove(nonNullTerminatedPath.data() + write,
                 nonNullTerminatedPath.data() + segment_begin, segment_size);
    write += segment_size;
  }

  if (write == 0) {
    nonNullTerminatedPath[write++] = '.';
  }
  if (trailing_separator &&
      !(write == root_size && absolute) &&
      nonNullTerminatedPath[write - 1] != '/') {
    nonNullTerminatedPath[write++] = '/';
  }

  assert(write <= nonNullTerminatedPath.size());
  return write;
}

} // namespace strobe::fs::details
