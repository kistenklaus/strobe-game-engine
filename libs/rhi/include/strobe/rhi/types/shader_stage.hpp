#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

enum class ShaderStage : uint32_t {
  none = 0,
  vertex = 1u << 0,
  tessellation_control = 1u << 1,
  tessellation_evaluation = 1u << 2,
  geometry = 1u << 3,
  fragment = 1u << 4,
  compute = 1u << 5,
  raygen = 1u << 6,
  anyhit = 1u << 7,
  hit = 1u << 8,
  miss = 1u << 9,
  intersection = 1u << 10,
  callable = 1u << 11,
  task = 1u << 12,
  mesh = 1u << 13,
};

[[nodiscard]]
constexpr ShaderStage operator|(ShaderStage lhs, ShaderStage rhs) noexcept {
  using T = std::underlying_type_t<ShaderStage>;
  return static_cast<ShaderStage>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ShaderStage operator&(ShaderStage lhs, ShaderStage rhs) noexcept {
  using T = std::underlying_type_t<ShaderStage>;
  return static_cast<ShaderStage>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ShaderStage operator^(ShaderStage lhs, ShaderStage rhs) noexcept {
  using T = std::underlying_type_t<ShaderStage>;
  return static_cast<ShaderStage>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

[[nodiscard]]
constexpr ShaderStage operator~(ShaderStage value) noexcept {
  using T = std::underlying_type_t<ShaderStage>;
  return static_cast<ShaderStage>(~static_cast<T>(value));
}

constexpr ShaderStage &operator|=(ShaderStage &lhs, ShaderStage rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr ShaderStage &operator&=(ShaderStage &lhs, ShaderStage rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr ShaderStage &operator^=(ShaderStage &lhs, ShaderStage rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool operator==(ShaderStage lhs,
                          std::underlying_type_t<ShaderStage> rhs) noexcept {
  return static_cast<std::underlying_type_t<ShaderStage>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(std::underlying_type_t<ShaderStage> lhs,
                          ShaderStage rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<ShaderStage>>(rhs);
}

[[nodiscard]]
constexpr bool operator!=(ShaderStage lhs,
                          std::underlying_type_t<ShaderStage> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(std::underlying_type_t<ShaderStage> lhs,
                          ShaderStage rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::rhi
