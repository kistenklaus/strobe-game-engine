#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class BuildFlags : uint32_t {
  none = 0,

  allow_update = 1u << 0,
  allow_compaction = 1u << 1,
  prefer_fast_trace = 1u << 2,
  prefer_fast_build = 1u << 3,
  low_memory = 1u << 4,
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
