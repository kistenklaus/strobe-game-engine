#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief Directory creation options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} mkdir_flags.hpp MkdirFlags
 */
// [MkdirFlags]
enum class MkdirFlags : std::uint32_t {
  none = 0,
  parents = 1u << 0,
};
// [MkdirFlags]

[[nodiscard]] constexpr MkdirFlags operator|(MkdirFlags lhs,
                                              MkdirFlags rhs) noexcept {
  using T = std::underlying_type_t<MkdirFlags>;
  return static_cast<MkdirFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr MkdirFlags operator&(MkdirFlags lhs,
                                              MkdirFlags rhs) noexcept {
  using T = std::underlying_type_t<MkdirFlags>;
  return static_cast<MkdirFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr MkdirFlags operator^(MkdirFlags lhs,
                                              MkdirFlags rhs) noexcept {
  using T = std::underlying_type_t<MkdirFlags>;
  return static_cast<MkdirFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr MkdirFlags operator~(MkdirFlags value) noexcept {
  using T = std::underlying_type_t<MkdirFlags>;
  return static_cast<MkdirFlags>(~static_cast<T>(value));
}

constexpr MkdirFlags &operator|=(MkdirFlags &lhs, MkdirFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr MkdirFlags &operator&=(MkdirFlags &lhs, MkdirFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr MkdirFlags &operator^=(MkdirFlags &lhs, MkdirFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
