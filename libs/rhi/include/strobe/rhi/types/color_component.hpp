#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class ColorComponent : uint8_t {
  none = 0,
  r = 1u << 0,
  g = 1u << 1,
  b = 1u << 2,
  a = 1u << 3,

  rgb = (1u << 0) | (1u << 1) | (1u << 2),
  rgba = (1u << 0) | (1u << 1) | (1u << 2) | (1u << 3),
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
