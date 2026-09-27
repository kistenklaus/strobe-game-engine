#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief File and directory move options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} mv_flags.hpp MvFlags
 */
// [MvFlags]
enum class MvFlags : std::uint32_t {
  none = 0,
  force = 1u << 0,
  preserve_timestamps = 1u << 1,
};
// [MvFlags]

[[nodiscard]] constexpr MvFlags operator|(MvFlags lhs, MvFlags rhs) noexcept {
  using T = std::underlying_type_t<MvFlags>;
  return static_cast<MvFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr MvFlags operator&(MvFlags lhs, MvFlags rhs) noexcept {
  using T = std::underlying_type_t<MvFlags>;
  return static_cast<MvFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr MvFlags operator^(MvFlags lhs, MvFlags rhs) noexcept {
  using T = std::underlying_type_t<MvFlags>;
  return static_cast<MvFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr MvFlags operator~(MvFlags value) noexcept {
  using T = std::underlying_type_t<MvFlags>;
  return static_cast<MvFlags>(~static_cast<T>(value));
}

constexpr MvFlags &operator|=(MvFlags &lhs, MvFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr MvFlags &operator&=(MvFlags &lhs, MvFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr MvFlags &operator^=(MvFlags &lhs, MvFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
