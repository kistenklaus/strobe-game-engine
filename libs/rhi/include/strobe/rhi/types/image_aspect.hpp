#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class ImageAspect : uint8_t {
  none = 0,
  color = 1u << 0,
  depth = 1u << 1,
  stencil = 1u << 2,
};

[[nodiscard]] constexpr ImageAspect operator|(ImageAspect lhs,
                                               ImageAspect rhs) noexcept {
  return static_cast<ImageAspect>(
      static_cast<std::underlying_type_t<ImageAspect>>(lhs) |
      static_cast<std::underlying_type_t<ImageAspect>>(rhs));
}

[[nodiscard]] constexpr ImageAspect operator&(ImageAspect lhs,
                                               ImageAspect rhs) noexcept {
  return static_cast<ImageAspect>(
      static_cast<std::underlying_type_t<ImageAspect>>(lhs) &
      static_cast<std::underlying_type_t<ImageAspect>>(rhs));
}

[[nodiscard]] constexpr ImageAspect operator^(ImageAspect lhs,
                                               ImageAspect rhs) noexcept {
  return static_cast<ImageAspect>(
      static_cast<std::underlying_type_t<ImageAspect>>(lhs) ^
      static_cast<std::underlying_type_t<ImageAspect>>(rhs));
}

[[nodiscard]] constexpr ImageAspect operator~(ImageAspect value) noexcept {
  return static_cast<ImageAspect>(
      ~static_cast<std::underlying_type_t<ImageAspect>>(value));
}

constexpr ImageAspect& operator|=(ImageAspect& lhs, ImageAspect rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ImageAspect& operator&=(ImageAspect& lhs, ImageAspect rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ImageAspect& operator^=(ImageAspect& lhs, ImageAspect rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::rhi
