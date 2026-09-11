#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Geometry build flags.
 *
 * Specifies per-geometry behavior for acceleration structure builds.
 * GeometryFlags values may be combined and queried using the provided bitwise
 * operators.
 */
enum class GeometryFlags : uint32_t {
  none = 0, ///< No additional geometry behavior.

  opaque = 1u << 0, ///< Treat geometry as opaque during ray traversal.
  no_duplicate_any_hit =
      1u << 1, ///< Prevent duplicate any-hit shader invocations.
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
