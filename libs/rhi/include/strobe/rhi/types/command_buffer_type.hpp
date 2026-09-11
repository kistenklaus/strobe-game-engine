#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Command buffer flags.
 *
 * Specifies command buffer allocation and recording behavior.
 * CommandBufferFlags values may be combined and queried using the provided
 * bitwise operators.
 */
enum class CommandBufferFlags : uint8_t {
  none      = 0,       ///< No additional behavior.
  secondary = 1u << 0, ///< Allocate a secondary command buffer.
  reusable  = 1u << 1, ///< Allow repeated submission of the command buffer.
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
