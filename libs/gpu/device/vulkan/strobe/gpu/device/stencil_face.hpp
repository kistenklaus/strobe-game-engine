#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::gpu {

enum class StencilFace : uint8_t {
  none = 0,
  front = 1u << 0,
  back = 1u << 1,
  front_and_back = (1u << 0) | (1u << 1),
};

[[nodiscard]]
constexpr StencilFace operator|(StencilFace lhs, StencilFace rhs) noexcept {
  using T = std::underlying_type_t<StencilFace>;

  return static_cast<StencilFace>(
      static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]]
constexpr StencilFace operator&(StencilFace lhs, StencilFace rhs) noexcept {
  using T = std::underlying_type_t<StencilFace>;

  return static_cast<StencilFace>(
      static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]]
constexpr StencilFace operator^(StencilFace lhs, StencilFace rhs) noexcept {
  using T = std::underlying_type_t<StencilFace>;

  return static_cast<StencilFace>(
      static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]]
constexpr StencilFace operator~(StencilFace value) noexcept {
  using T = std::underlying_type_t<StencilFace>;

  return static_cast<StencilFace>(~static_cast<T>(value));
}

constexpr StencilFace &
operator|=(StencilFace &lhs, StencilFace rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr StencilFace &
operator&=(StencilFace &lhs, StencilFace rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr StencilFace &
operator^=(StencilFace &lhs, StencilFace rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(
    StencilFace lhs,
    std::underlying_type_t<StencilFace> rhs) noexcept {

  return static_cast<std::underlying_type_t<StencilFace>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(
    std::underlying_type_t<StencilFace> lhs,
    StencilFace rhs) noexcept {

  return lhs == static_cast<std::underlying_type_t<StencilFace>>(rhs);
}

[[nodiscard]]
constexpr bool operator!=(
    StencilFace lhs,
    std::underlying_type_t<StencilFace> rhs) noexcept {

  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(
    std::underlying_type_t<StencilFace> lhs,
    StencilFace rhs) noexcept {

  return !(lhs == rhs);
}

} // namespace strobe::gpu
