#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class GeometryFlags : uint32_t {
  none = 0,

  opaque = 1u << 0,
  no_duplicate_any_hit = 1u << 1,
};

[[nodiscard]] constexpr GeometryFlags
operator|(GeometryFlags lhs, GeometryFlags rhs) noexcept {
  return static_cast<GeometryFlags>(
      static_cast<std::underlying_type_t<GeometryFlags>>(lhs) |
      static_cast<std::underlying_type_t<GeometryFlags>>(rhs));
}

[[nodiscard]] constexpr GeometryFlags
operator&(GeometryFlags lhs, GeometryFlags rhs) noexcept {
  return static_cast<GeometryFlags>(
      static_cast<std::underlying_type_t<GeometryFlags>>(lhs) &
      static_cast<std::underlying_type_t<GeometryFlags>>(rhs));
}

[[nodiscard]] constexpr GeometryFlags
operator^(GeometryFlags lhs, GeometryFlags rhs) noexcept {
  return static_cast<GeometryFlags>(
      static_cast<std::underlying_type_t<GeometryFlags>>(lhs) ^
      static_cast<std::underlying_type_t<GeometryFlags>>(rhs));
}

[[nodiscard]] constexpr GeometryFlags
operator~(GeometryFlags value) noexcept {
  return static_cast<GeometryFlags>(
      ~static_cast<std::underlying_type_t<GeometryFlags>>(value));
}

constexpr GeometryFlags &
operator|=(GeometryFlags &lhs, GeometryFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr GeometryFlags &
operator&=(GeometryFlags &lhs, GeometryFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr GeometryFlags &
operator^=(GeometryFlags &lhs, GeometryFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool
operator==(GeometryFlags lhs,
           std::underlying_type_t<GeometryFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<GeometryFlags>>(lhs) == rhs;
}

[[nodiscard]] constexpr bool
operator!=(GeometryFlags lhs,
           std::underlying_type_t<GeometryFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<GeometryFlags>>(lhs) != rhs;
}

[[nodiscard]] constexpr bool
operator==(std::underlying_type_t<GeometryFlags> lhs,
           GeometryFlags rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<GeometryFlags>>(rhs);
}

[[nodiscard]] constexpr bool
operator!=(std::underlying_type_t<GeometryFlags> lhs,
           GeometryFlags rhs) noexcept {
  return lhs != static_cast<std::underlying_type_t<GeometryFlags>>(rhs);
}

} // namespace strobe::rhi
