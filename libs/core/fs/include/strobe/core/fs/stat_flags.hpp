#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief Filesystem status query options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} stat_flags.hpp StatFlags
 */
// [StatFlags]
enum class StatFlags : std::uint32_t {
  none = 0,
  follow_symlink = 1u << 0,
};
// [StatFlags]

[[nodiscard]] constexpr StatFlags operator|(StatFlags lhs,
                                             StatFlags rhs) noexcept {
  using T = std::underlying_type_t<StatFlags>;
  return static_cast<StatFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr StatFlags operator&(StatFlags lhs,
                                             StatFlags rhs) noexcept {
  using T = std::underlying_type_t<StatFlags>;
  return static_cast<StatFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr StatFlags operator^(StatFlags lhs,
                                             StatFlags rhs) noexcept {
  using T = std::underlying_type_t<StatFlags>;
  return static_cast<StatFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr StatFlags operator~(StatFlags value) noexcept {
  using T = std::underlying_type_t<StatFlags>;
  return static_cast<StatFlags>(~static_cast<T>(value));
}

constexpr StatFlags &operator|=(StatFlags &lhs, StatFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr StatFlags &operator&=(StatFlags &lhs, StatFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr StatFlags &operator^=(StatFlags &lhs, StatFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
