#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief File and directory removal options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} rm_flags.hpp RmFlags
 */
// [RmFlags]
enum class RmFlags : std::uint32_t {
  none = 0,
  recursive = 1u << 0,
  force = 1u << 1,
};
// [RmFlags]

[[nodiscard]] constexpr RmFlags operator|(RmFlags lhs, RmFlags rhs) noexcept {
  using T = std::underlying_type_t<RmFlags>;
  return static_cast<RmFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr RmFlags operator&(RmFlags lhs, RmFlags rhs) noexcept {
  using T = std::underlying_type_t<RmFlags>;
  return static_cast<RmFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr RmFlags operator^(RmFlags lhs, RmFlags rhs) noexcept {
  using T = std::underlying_type_t<RmFlags>;
  return static_cast<RmFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr RmFlags operator~(RmFlags value) noexcept {
  using T = std::underlying_type_t<RmFlags>;
  return static_cast<RmFlags>(~static_cast<T>(value));
}

constexpr RmFlags &operator|=(RmFlags &lhs, RmFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr RmFlags &operator&=(RmFlags &lhs, RmFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr RmFlags &operator^=(RmFlags &lhs, RmFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
