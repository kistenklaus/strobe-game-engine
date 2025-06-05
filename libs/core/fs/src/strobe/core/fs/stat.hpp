#pragma once

#include "strobe/core/fs/Path.hpp"
#include <sys/stat.h> // stat()

namespace strobe::fs {

enum class StatFlagBits {
  None = 0,
  FollowSymLink = 1 << 0,
};

inline constexpr StatFlagBits operator|(StatFlagBits lhs, StatFlagBits rhs) {
  using T = std::underlying_type_t<StatFlagBits>;
  return static_cast<StatFlagBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline constexpr StatFlagBits operator&(StatFlagBits lhs, StatFlagBits rhs) {
  using T = std::underlying_type_t<StatFlagBits>;
  return static_cast<StatFlagBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline constexpr StatFlagBits &operator|=(StatFlagBits &lhs, StatFlagBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline constexpr StatFlagBits &operator&=(StatFlagBits &lhs, StatFlagBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

struct StatFlags {
  StatFlagBits bits;
  constexpr StatFlags(StatFlagBits bits = StatFlagBits::None) : bits(bits) {}

  constexpr operator bool() const {
    using T = std::underlying_type_t<StatFlagBits>;
    return static_cast<T>(bits) != 0;
  }

  constexpr operator StatFlagBits() const { return bits; }
};

inline constexpr StatFlags operator|(StatFlags lhs, StatFlagBits rhs) {
  return StatFlags(lhs.bits | rhs);
}

inline constexpr StatFlags operator&(StatFlags lhs, StatFlagBits rhs) {
  return StatFlags(lhs.bits & rhs);
}

enum class Type { File, Directory, SymLink };

struct Stat;

Stat stat(PathView path, StatFlags flags = StatFlagBits::None);
Stat stat(int path, StatFlags flags = StatFlagBits::None);

struct Stat {
public:
  friend Stat stat(PathView, StatFlags);

  bool isFile() const { return m_type == Type::File; }
  bool isDirectory() const { return m_type == Type::Directory; }
  Type type() const { return m_type; }
  std::size_t size() const { return m_size; }

private:
  Stat() = default;
  Type m_type;
  std::size_t m_size;
};

} // namespace strobe::fs
