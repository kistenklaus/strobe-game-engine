#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class CommandBufferFlags : uint8_t {
  none      = 0,
  secondary = 1u << 0,
  reusable  = 1u << 1,
};

[[nodiscard]]
constexpr CommandBufferFlags
operator|(CommandBufferFlags lhs, CommandBufferFlags rhs) noexcept {
  using T = std::underlying_type_t<CommandBufferFlags>;

  return static_cast<CommandBufferFlags>(
      static_cast<T>(lhs) |
      static_cast<T>(rhs));
}

[[nodiscard]]
constexpr CommandBufferFlags
operator&(CommandBufferFlags lhs, CommandBufferFlags rhs) noexcept {
  using T = std::underlying_type_t<CommandBufferFlags>;

  return static_cast<CommandBufferFlags>(
      static_cast<T>(lhs) &
      static_cast<T>(rhs));
}

constexpr CommandBufferFlags &
operator|=(CommandBufferFlags &lhs, CommandBufferFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CommandBufferFlags &
operator&=(CommandBufferFlags &lhs, CommandBufferFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(CommandBufferFlags lhs,
           std::underlying_type_t<CommandBufferFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<CommandBufferFlags>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool
operator!=(CommandBufferFlags lhs,
           std::underlying_type_t<CommandBufferFlags> rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
