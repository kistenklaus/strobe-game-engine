#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::fs {

/**
 * \brief File and directory copy options.
 * \ingroup core
 *
 * Values may be combined with the bitwise operators.
 * \snippet{.cpp} copy_flags.hpp CpFlags
 */
// [CpFlags]
enum class CpFlags : std::uint32_t {
  none = 0,
  recursive = 1u << 0,
  preserve = 1u << 1,
};
// [CpFlags]

[[nodiscard]] constexpr CpFlags operator|(CpFlags lhs, CpFlags rhs) noexcept {
  using T = std::underlying_type_t<CpFlags>;
  return static_cast<CpFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]] constexpr CpFlags operator&(CpFlags lhs, CpFlags rhs) noexcept {
  using T = std::underlying_type_t<CpFlags>;
  return static_cast<CpFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]] constexpr CpFlags operator^(CpFlags lhs, CpFlags rhs) noexcept {
  using T = std::underlying_type_t<CpFlags>;
  return static_cast<CpFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]] constexpr CpFlags operator~(CpFlags value) noexcept {
  using T = std::underlying_type_t<CpFlags>;
  return static_cast<CpFlags>(~static_cast<T>(value));
}

constexpr CpFlags &operator|=(CpFlags &lhs, CpFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CpFlags &operator&=(CpFlags &lhs, CpFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr CpFlags &operator^=(CpFlags &lhs, CpFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::fs
