#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image aspect mask.
 *
 * Selects the image aspects affected by an operation. ImageAspect values may
 * be combined using the provided bitwise operators.
 */
enum class ImageAspect : uint8_t {
  none = 0,          ///< No image aspects.
  color = 1u << 0,  ///< Color aspect.
  depth = 1u << 1,  ///< Depth aspect.
  stencil = 1u << 2,///< Stencil aspect.
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

constexpr ImageAspect &operator|=(ImageAspect &lhs, ImageAspect rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ImageAspect &operator&=(ImageAspect &lhs, ImageAspect rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ImageAspect &operator^=(ImageAspect &lhs, ImageAspect rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

} // namespace strobe::rhi
