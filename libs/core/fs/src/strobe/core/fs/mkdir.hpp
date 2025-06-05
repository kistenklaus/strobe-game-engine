#pragma once

#include "Path.hpp"
#include <strobe/core/memory/AllocatorTraits.hpp>

namespace strobe::fs {

enum class MkdirFlagBits {
  None = 0,
  Parents = 1 << 0,
};

inline constexpr MkdirFlagBits operator|(MkdirFlagBits lhs, MkdirFlagBits rhs) {
  using T = std::underlying_type_t<MkdirFlagBits>;
  return static_cast<MkdirFlagBits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline constexpr MkdirFlagBits operator&(MkdirFlagBits lhs, MkdirFlagBits rhs) {
  using T = std::underlying_type_t<MkdirFlagBits>;
  return static_cast<MkdirFlagBits>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline constexpr MkdirFlagBits &operator|=(MkdirFlagBits &lhs,
                                           MkdirFlagBits rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline constexpr MkdirFlagBits &operator&=(MkdirFlagBits &lhs,
                                           MkdirFlagBits rhs) {
  lhs = lhs & rhs;
  return lhs;
}

struct MkdirFlags {
  MkdirFlagBits bits;
  constexpr MkdirFlags(MkdirFlagBits bits = MkdirFlagBits::None) : bits(bits) {}

  constexpr operator bool() const {
    using T = std::underlying_type_t<MkdirFlagBits>;
    return static_cast<T>(bits) != 0;
  }

  constexpr operator MkdirFlagBits() const { return bits; }
};

inline constexpr MkdirFlags operator|(MkdirFlags lhs, MkdirFlagBits rhs) {
  return MkdirFlags(lhs.bits | rhs);
}

inline constexpr MkdirFlags operator&(MkdirFlags lhs, MkdirFlagBits rhs) {
  return MkdirFlags(lhs.bits & rhs);
}

void mkdir(PathView path, MkdirFlags flags = MkdirFlagBits::None);

template <Allocator PA> void mkdir(const Path<PA> &path, MkdirFlags flags = MkdirFlagBits::None) {
  mkdir(PathView(path), flags);
}

} // namespace strobe::fs
