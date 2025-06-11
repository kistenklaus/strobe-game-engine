#pragma once

#include "strobe/core/fs/Path.hpp"
#include <type_traits>
namespace strobe::fs {

enum class MvFlagBits {
  None = 0,
  Force = 1 << 0, // overwrite 
  PreserveTimestamps = 1 << 1 // preserve timestamps.
};

constexpr MvFlagBits operator|(MvFlagBits lhs, MvFlagBits rhs) {
  using T = std::underlying_type_t<MvFlagBits>;
  return static_cast<MvFlagBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

constexpr MvFlagBits operator&(MvFlagBits lhs, MvFlagBits rhs) {
  using T = std::underlying_type_t<MvFlagBits>;
  return static_cast<MvFlagBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

constexpr MvFlagBits &operator|=(MvFlagBits &lhs, MvFlagBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

constexpr MvFlagBits &operator&=(MvFlagBits &lhs, MvFlagBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

constexpr MvFlagBits operator~(MvFlagBits rhs) {
  using T = std::underlying_type_t<MvFlagBits>;
  return static_cast<MvFlagBits>(~static_cast<T>(rhs));
}

struct MvFlags {
  MvFlagBits bits;

  constexpr MvFlags(MvFlagBits bits) : bits(bits) {}

  constexpr operator bool() const {
    return std::underlying_type_t<MvFlagBits>(bits) != 0;
  }

  constexpr operator MvFlagBits() const { return bits; }
};

constexpr MvFlags operator|(MvFlags lhs, MvFlagBits rhs) {
  return MvFlags(lhs.bits | rhs);
}

constexpr MvFlags operator&(MvFlags lhs, MvFlagBits rhs) {
  return MvFlags(lhs.bits & rhs);
}

void mv(PathView src, PathView dst, MvFlags flags = MvFlagBits::None);

} // namespace strobe::fs
  //
