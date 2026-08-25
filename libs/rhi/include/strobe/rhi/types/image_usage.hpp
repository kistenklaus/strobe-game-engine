#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class ImageUsage : uint32_t {
  none = 0,

  transfer_src = 1u << 0,
  transfer_dst = 1u << 1,

  sampled = 1u << 2,
  storage = 1u << 3,

  color_attachment = 1u << 4,
  depth_stencil = 1u << 5,
};

[[nodiscard]]
constexpr ImageUsage operator|(ImageUsage lhs, ImageUsage rhs) noexcept {
  return static_cast<ImageUsage>(
      static_cast<std::underlying_type_t<ImageUsage>>(lhs) |
      static_cast<std::underlying_type_t<ImageUsage>>(rhs));
}

[[nodiscard]]
constexpr ImageUsage operator&(ImageUsage lhs, ImageUsage rhs) noexcept {
  return static_cast<ImageUsage>(
      static_cast<std::underlying_type_t<ImageUsage>>(lhs) &
      static_cast<std::underlying_type_t<ImageUsage>>(rhs));
}

[[nodiscard]]
constexpr ImageUsage operator^(ImageUsage lhs, ImageUsage rhs) noexcept {
  return static_cast<ImageUsage>(
      static_cast<std::underlying_type_t<ImageUsage>>(lhs) ^
      static_cast<std::underlying_type_t<ImageUsage>>(rhs));
}

[[nodiscard]]
constexpr ImageUsage operator~(ImageUsage value) noexcept {
  return static_cast<ImageUsage>(
      ~static_cast<std::underlying_type_t<ImageUsage>>(value));
}

constexpr ImageUsage &operator|=(ImageUsage &lhs, ImageUsage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ImageUsage &operator&=(ImageUsage &lhs, ImageUsage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ImageUsage &operator^=(ImageUsage &lhs, ImageUsage rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(ImageUsage lhs,
           std::underlying_type_t<ImageUsage> rhs) noexcept {
  return static_cast<std::underlying_type_t<ImageUsage>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool
operator==(std::underlying_type_t<ImageUsage> lhs,
           ImageUsage rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<ImageUsage>>(rhs);
}

[[nodiscard]]
constexpr bool
operator!=(ImageUsage lhs,
           std::underlying_type_t<ImageUsage> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool
operator!=(std::underlying_type_t<ImageUsage> lhs,
           ImageUsage rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
