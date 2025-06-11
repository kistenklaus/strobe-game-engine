#pragma once

#include "strobe/core/fs/Path.hpp"
namespace strobe::fs {

enum class CpFlagBits {
  None = 0,
  Recursive = 1 << 0,
  Preserve = 1 << 1 // preserve timestamps.
};

constexpr CpFlagBits operator|(CpFlagBits lhs, CpFlagBits rhs) {
  using T = std::underlying_type_t<CpFlagBits>;
  return static_cast<CpFlagBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

constexpr CpFlagBits operator&(CpFlagBits lhs, CpFlagBits rhs) {
  using T = std::underlying_type_t<CpFlagBits>;
  return static_cast<CpFlagBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

constexpr CpFlagBits &operator|=(CpFlagBits &lhs, CpFlagBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CpFlagBits &operator&=(CpFlagBits &lhs, CpFlagBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

constexpr CpFlagBits operator~(CpFlagBits rhs) {
  using T = std::underlying_type_t<CpFlagBits>;
  return static_cast<CpFlagBits>(~static_cast<T>(rhs));
}

struct CpFlags {
  CpFlagBits bits;

  constexpr CpFlags(CpFlagBits bits) : bits(bits) {}

  constexpr operator bool() const {
    return std::underlying_type_t<CpFlagBits>(bits) != 0;
  }

  constexpr operator CpFlagBits() const { return bits; }
};

constexpr CpFlags operator|(CpFlags lhs, CpFlagBits rhs) {
  return CpFlags(lhs.bits | rhs);
}

constexpr CpFlags operator&(CpFlags lhs, CpFlagBits rhs) {
  return CpFlags(lhs.bits & rhs);
}

void cp(PathView src, PathView dst, CpFlags flags = CpFlagBits::None);

} // namespace strobe::fs
