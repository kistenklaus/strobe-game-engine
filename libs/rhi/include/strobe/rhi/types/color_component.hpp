#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Color component mask.
 *
 * Selects color components for operations such as attachment write masks.
 * ColorComponent values may be combined and queried using the provided bitwise
 * operators.
 */
enum class ColorComponent : uint8_t {
  none = 0, ///< No color components.
  r = 1u << 0, ///< Red component.
  g = 1u << 1, ///< Green component.
  b = 1u << 2, ///< Blue component.
  a = 1u << 3, ///< Alpha component.

  rgb = (1u << 0) | (1u << 1) | (1u << 2), ///< Red, green, and blue.
  rgba = (1u << 0) | (1u << 1) | (1u << 2) | (1u << 3), ///< All components.
};

[[nodiscard]]
constexpr ColorComponent operator|(
    ColorComponent lhs,
    ColorComponent rhs) noexcept {
  using T = std::underlying_type_t<ColorComponent>;

  return static_cast<ColorComponent>(
      static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ColorComponent operator&(
    ColorComponent lhs,
    ColorComponent rhs) noexcept {
  using T = std::underlying_type_t<ColorComponent>;

  return static_cast<ColorComponent>(
      static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ColorComponent operator^(
    ColorComponent lhs,
    ColorComponent rhs) noexcept {
  using T = std::underlying_type_t<ColorComponent>;

  return static_cast<ColorComponent>(
      static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ColorComponent operator~(
    ColorComponent value) noexcept {
  using T = std::underlying_type_t<ColorComponent>;

  return static_cast<ColorComponent>(
      ~static_cast<T>(value));
}

constexpr ColorComponent &operator|=(
    ColorComponent &lhs,
    ColorComponent rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ColorComponent &operator&=(
    ColorComponent &lhs,
    ColorComponent rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ColorComponent &operator^=(
    ColorComponent &lhs,
    ColorComponent rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(
    ColorComponent lhs,
    std::underlying_type_t<ColorComponent> rhs) noexcept {
  return static_cast<std::underlying_type_t<ColorComponent>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(
    std::underlying_type_t<ColorComponent> lhs,
    ColorComponent rhs) noexcept {
  return lhs ==
         static_cast<std::underlying_type_t<ColorComponent>>(rhs);
}

[[nodiscard]]
constexpr bool operator!=(
    ColorComponent lhs,
    std::underlying_type_t<ColorComponent> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(
    std::underlying_type_t<ColorComponent> lhs,
    ColorComponent rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
