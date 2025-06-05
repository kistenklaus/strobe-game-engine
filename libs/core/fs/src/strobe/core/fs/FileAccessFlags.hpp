#pragma once

#include <type_traits>

namespace strobe {

enum class FileAccessBits : unsigned int {
  Read = 1 << 0,
  Write = 1 << 1,
  ReadWrite = Read | Write,
  Create = 1 << 2,
  Trunc = 1 << 3,
  Append = 1 << 4,
  Exclusive = 1 << 5,
  Sync = 1 << 6,
};

constexpr FileAccessBits operator|(FileAccessBits lhs, FileAccessBits rhs) {
  using T = std::underlying_type_t<FileAccessBits>;
  return static_cast<FileAccessBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

constexpr FileAccessBits operator&(FileAccessBits lhs, FileAccessBits rhs) {
  using T = std::underlying_type_t<FileAccessBits>;
  return static_cast<FileAccessBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

constexpr FileAccessBits &operator|=(FileAccessBits &lhs, FileAccessBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

constexpr FileAccessBits &operator&=(FileAccessBits &lhs, FileAccessBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

constexpr FileAccessBits operator~(FileAccessBits rhs) {
  using T = std::underlying_type_t<FileAccessBits>;
  return static_cast<FileAccessBits>(~static_cast<T>(rhs));
}

struct FileAccessFlags {
  FileAccessBits bits;

  constexpr FileAccessFlags(FileAccessBits bits) : bits(bits) {}

  constexpr operator bool() const {
    using T = std::underlying_type_t<FileAccessBits>;
    return static_cast<T>(bits) != 0;
  }

  constexpr operator FileAccessBits() const { return bits; }
  // Add operator| etc. forwarding here
};

constexpr FileAccessFlags operator|(FileAccessFlags lhs, FileAccessBits rhs) {
  return FileAccessFlags(lhs.bits | rhs);
}

constexpr FileAccessFlags operator&(FileAccessFlags lhs, FileAccessBits rhs) {
  return FileAccessFlags(lhs.bits & rhs);
}

} // namespace strobe
