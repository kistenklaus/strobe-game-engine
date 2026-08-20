#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::gpu {


enum class MemoryAllocationFlags : uint8_t {
  none = 0,
  prefer_dedicated = 1u << 0,
  require_dedicated = 1u << 1,
};

[[nodiscard]]
constexpr MemoryAllocationFlags operator|(MemoryAllocationFlags lhs,
                                          MemoryAllocationFlags rhs) noexcept {
  using T = std::underlying_type_t<MemoryAllocationFlags>;

  return static_cast<MemoryAllocationFlags>(static_cast<T>(lhs) |
                                            static_cast<T>(rhs));
}

[[nodiscard]]
constexpr MemoryAllocationFlags operator&(MemoryAllocationFlags lhs,
                                          MemoryAllocationFlags rhs) noexcept {
  using T = std::underlying_type_t<MemoryAllocationFlags>;

  return static_cast<MemoryAllocationFlags>(static_cast<T>(lhs) &
                                            static_cast<T>(rhs));
}

constexpr MemoryAllocationFlags &
operator|=(MemoryAllocationFlags &lhs, MemoryAllocationFlags rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr MemoryAllocationFlags &
operator&=(MemoryAllocationFlags &lhs, MemoryAllocationFlags rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(MemoryAllocationFlags lhs,
           std::underlying_type_t<MemoryAllocationFlags> rhs) noexcept {
  return static_cast<std::underlying_type_t<MemoryAllocationFlags>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool
operator!=(MemoryAllocationFlags lhs,
           std::underlying_type_t<MemoryAllocationFlags> rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::gpu
