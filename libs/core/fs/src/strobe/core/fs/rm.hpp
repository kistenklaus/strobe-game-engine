#pragma once

#include "fmt/format.h"
#include "strobe/core/fs/Path.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/stat.hpp"
#include <type_traits>

namespace strobe::fs {

enum class RmFlagBits {
  None = 0,
  Recursive = 1 << 0,
  Force = 1 << 1,
};
inline constexpr RmFlagBits operator|(RmFlagBits lhs, RmFlagBits rhs) {
  using T = std::underlying_type_t<RmFlagBits>;
  return static_cast<RmFlagBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline constexpr RmFlagBits operator&(RmFlagBits lhs, RmFlagBits rhs) {
  using T = std::underlying_type_t<RmFlagBits>;
  return static_cast<RmFlagBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline constexpr RmFlagBits operator~(RmFlagBits rhs) {
  using T = std::underlying_type_t<RmFlagBits>;
  return static_cast<RmFlagBits>(~static_cast<T>(rhs));
}

inline constexpr RmFlagBits &operator|=(RmFlagBits &lhs, RmFlagBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline constexpr RmFlagBits &operator&=(RmFlagBits &lhs, RmFlagBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

struct RmFlags {
  RmFlagBits bits;
  constexpr RmFlags(RmFlagBits bits) : bits(bits) {}

  constexpr operator bool() const {
    return static_cast<std::underlying_type_t<RmFlagBits>>(bits) != 0;
  }

  constexpr operator RmFlagBits() const { return bits; }
};

inline constexpr RmFlags operator|(RmFlags lhs, RmFlagBits rhs) {
  return RmFlags(lhs.bits | rhs);
}

inline constexpr RmFlags operator&(RmFlags lhs, RmFlagBits rhs) {
  return RmFlags(lhs.bits & rhs);
}

inline constexpr RmFlags operator~(RmFlags lhs) { return RmFlags(~lhs.bits); }

inline constexpr RmFlags &operator|=(RmFlags &lhs, RmFlagBits rhs) {
  lhs.bits |= rhs;
  return lhs;
}

inline constexpr RmFlags &operator&=(RmFlags &lhs, RmFlagBits rhs) {
  lhs.bits &= rhs;
  return lhs;
}

void rm(PathView path, const Stat *stat, RmFlags flags = RmFlagBits::None);

inline void rm(PathView path, RmFlags flags = RmFlagBits::None) {
  if (!strobe::fs::exists(path)) {
    if (!(flags & RmFlagBits::Force)) {
      throw std::invalid_argument(fmt::format(
          "Cannot remove '{}': No such file or directory", path.c_str()));
    }
    return;
  }

  Stat stat = strobe::fs::stat(path);
  rm(path, &stat, flags);
}

} // namespace strobe::fs
