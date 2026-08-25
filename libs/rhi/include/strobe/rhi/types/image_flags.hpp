#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class ImageFlags : uint32_t {
  none = 0,
  mutable_format = 1u << 0,
  cube_compatible = 1u << 1,
};

[[nodiscard]]
constexpr ImageFlags operator|(ImageFlags lhs, ImageFlags rhs) noexcept {
  return static_cast<ImageFlags>(
      static_cast<std::underlying_type_t<ImageFlags>>(lhs) |
      static_cast<std::underlying_type_t<ImageFlags>>(rhs));
}

[[nodiscard]]
constexpr ImageFlags operator&(ImageFlags lhs, ImageFlags rhs) noexcept {
  return static_cast<ImageFlags>(
      static_cast<std::underlying_type_t<ImageFlags>>(lhs) &
      static_cast<std::underlying_type_t<ImageFlags>>(rhs));
}

[[nodiscard]]
constexpr ImageFlags operator^(ImageFlags lhs, ImageFlags rhs) noexcept {
  return static_cast<ImageFlags>(
      static_cast<std::underlying_type_t<ImageFlags>>(lhs) ^
      static_cast<std::underlying_type_t<ImageFlags>>(rhs));
}

[[nodiscard]]
constexpr ImageFlags operator~(ImageFlags value) noexcept {
  return static_cast<ImageFlags>(
      ~static_cast<std::underlying_type_t<ImageFlags>>(value));
}

constexpr ImageFlags &operator|=(ImageFlags &lhs, ImageFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ImageFlags &operator&=(ImageFlags &lhs, ImageFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ImageFlags &operator^=(ImageFlags &lhs, ImageFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(ImageFlags lhs,
                          std::underlying_type_t<ImageFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<ImageFlags>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(std::underlying_type_t<ImageFlags> lhs,
                          ImageFlags rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<ImageFlags>>(rhs);
}

[[nodiscard]]
constexpr bool operator!=(ImageFlags lhs,
                          std::underlying_type_t<ImageFlags> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(std::underlying_type_t<ImageFlags> lhs,
                          ImageFlags rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
