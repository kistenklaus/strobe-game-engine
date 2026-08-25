#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class CullMode : uint8_t {
  none = 0,
  front = 1u << 0,
  back = 1u << 1,
  front_and_back = front | back,
};

[[nodiscard]]
constexpr CullMode operator|(CullMode lhs, CullMode rhs) noexcept {
  using T = std::underlying_type_t<CullMode>;

  return static_cast<CullMode>(
      static_cast<T>(lhs) |
      static_cast<T>(rhs));
}

[[nodiscard]]
constexpr CullMode operator&(CullMode lhs, CullMode rhs) noexcept {
  using T = std::underlying_type_t<CullMode>;

  return static_cast<CullMode>(
      static_cast<T>(lhs) &
      static_cast<T>(rhs));
}

constexpr CullMode &operator|=(CullMode &lhs, CullMode rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CullMode &operator&=(CullMode &lhs, CullMode rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(CullMode lhs,
                          std::underlying_type_t<CullMode> rhs) noexcept {
  return static_cast<std::underlying_type_t<CullMode>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator!=(CullMode lhs,
                          std::underlying_type_t<CullMode> rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
