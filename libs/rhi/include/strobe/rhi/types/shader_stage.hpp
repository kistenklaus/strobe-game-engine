#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Shader stage mask.
 *
 * Specifies shader stages participating in an operation. ShaderStage values
 * may be combined and queried using the provided bitwise operators.
 */
enum class ShaderStage : uint32_t {
  none = 0, ///< No shader stages.

  vertex = 1u << 0, ///< Vertex shader stage.
  tessellation_control = 1u << 1, ///< Tessellation control shader stage.
  tessellation_evaluation = 1u << 2, ///< Tessellation evaluation shader stage.
  geometry = 1u << 3, ///< Geometry shader stage.
  fragment = 1u << 4, ///< Fragment shader stage.
  compute = 1u << 5, ///< Compute shader stage.

  raygen = 1u << 6, ///< Ray generation shader stage.
  anyhit = 1u << 7, ///< Any-hit shader stage.
  hit = 1u << 8, ///< Closest-hit shader stage.
  miss = 1u << 9, ///< Miss shader stage.
  intersection = 1u << 10, ///< Intersection shader stage.
  callable = 1u << 11, ///< Callable shader stage.

  task = 1u << 12, ///< Task shader stage.
  mesh = 1u << 13, ///< Mesh shader stage.
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
