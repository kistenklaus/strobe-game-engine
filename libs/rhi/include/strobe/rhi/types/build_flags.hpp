#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Acceleration structure build flags.
 *
 * Specifies build behavior and optimization preferences for acceleration
 * structures. BuildFlags values may be combined and queried using the provided
 * bitwise operators.
 */
enum class BuildFlags : uint32_t {
  none = 0, ///< No additional build behavior.

  allow_update = 1u << 0, ///< Allows subsequent acceleration structure updates.
  allow_compaction = 1u << 1, ///< Allows acceleration structure compaction.
  prefer_fast_trace = 1u << 2, ///< Prefer faster traversal performance.
  prefer_fast_build = 1u << 3, ///< Prefer faster build performance.
  low_memory = 1u << 4, ///< Prefer reduced memory usage.
};

[[nodiscard]] constexpr BuildFlags
operator|(BuildFlags lhs,
          BuildFlags rhs) noexcept {
  return static_cast<BuildFlags>(
      static_cast<std::underlying_type_t<BuildFlags>>(
          lhs) |
      static_cast<std::underlying_type_t<BuildFlags>>(
          rhs));
}

[[nodiscard]] constexpr BuildFlags
operator&(BuildFlags lhs,
          BuildFlags rhs) noexcept {
  return static_cast<BuildFlags>(
      static_cast<std::underlying_type_t<BuildFlags>>(
          lhs) &
      static_cast<std::underlying_type_t<BuildFlags>>(
          rhs));
}

[[nodiscard]] constexpr BuildFlags
operator^(BuildFlags lhs,
          BuildFlags rhs) noexcept {
  return static_cast<BuildFlags>(
      static_cast<std::underlying_type_t<BuildFlags>>(
          lhs) ^
      static_cast<std::underlying_type_t<BuildFlags>>(
          rhs));
}

[[nodiscard]] constexpr BuildFlags
operator~(BuildFlags value) noexcept {
  return static_cast<BuildFlags>(
      ~static_cast<std::underlying_type_t<BuildFlags>>(
          value));
}

constexpr BuildFlags &
operator|=(BuildFlags &lhs,
           BuildFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr BuildFlags &
operator&=(BuildFlags &lhs,
           BuildFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr BuildFlags &
operator^=(BuildFlags &lhs,
           BuildFlags rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]] constexpr bool operator==(
    BuildFlags lhs,
    std::underlying_type_t<BuildFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<BuildFlags>>(
             lhs) == rhs;
}

[[nodiscard]] constexpr bool operator!=(
    BuildFlags lhs,
    std::underlying_type_t<BuildFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<BuildFlags>>(
             lhs) != rhs;
}

[[nodiscard]] constexpr bool
operator==(std::underlying_type_t<BuildFlags> lhs,
           BuildFlags rhs) noexcept {
  return lhs ==
         static_cast<std::underlying_type_t<BuildFlags>>(
             rhs);
}

[[nodiscard]] constexpr bool
operator!=(std::underlying_type_t<BuildFlags> lhs,
           BuildFlags rhs) noexcept {
  return lhs !=
         static_cast<std::underlying_type_t<BuildFlags>>(
             rhs);
}

} // namespace strobe::rhi
