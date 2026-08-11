#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::gpu::vulkan {

enum class FragmentDynamicState : uint64_t {
  none = 0,
  depth_test_enable = 1u << 0,
  depth_write_enable = 1u << 1,
  depth_compare_op = 1u << 2,
  depth_bounds_test = 1u << 3,
  depth_bounds = 1u << 4,
  stencil_test_enable = 1u << 5,
  stencil_op = 1u << 6,
  stencil_compare_mask = 1u << 7,
  stencil_write_mask = 1u << 8,
  stencil_reference = 1u << 9,
};

enum class VertexDynamicState : uint32_t {
  none = 0,
  viewport_count = 1u << 0,
  line_width = 1u << 1,
  depth_bias = 1u << 2,
  cull_mode = 1u << 3,
  front_face = 1u << 4,
  rasterizer_discard = 1u << 5,
  depth_bias_enable = 1u << 6,
  patch_control_points = 1u << 7,
};

[[nodiscard]]
constexpr FragmentDynamicState operator|(FragmentDynamicState lhs,
                                         FragmentDynamicState rhs) noexcept {
  return static_cast<FragmentDynamicState>(
      static_cast<std::underlying_type_t<FragmentDynamicState>>(lhs) |
      static_cast<std::underlying_type_t<FragmentDynamicState>>(rhs));
}

[[nodiscard]]
constexpr FragmentDynamicState operator&(FragmentDynamicState lhs,
                                         FragmentDynamicState rhs) noexcept {
  return static_cast<FragmentDynamicState>(
      static_cast<std::underlying_type_t<FragmentDynamicState>>(lhs) &
      static_cast<std::underlying_type_t<FragmentDynamicState>>(rhs));
}

[[nodiscard]]
constexpr FragmentDynamicState operator^(FragmentDynamicState lhs,
                                         FragmentDynamicState rhs) noexcept {
  return static_cast<FragmentDynamicState>(
      static_cast<std::underlying_type_t<FragmentDynamicState>>(lhs) ^
      static_cast<std::underlying_type_t<FragmentDynamicState>>(rhs));
}

[[nodiscard]]
constexpr FragmentDynamicState operator~(FragmentDynamicState value) noexcept {
  return static_cast<FragmentDynamicState>(
      ~static_cast<std::underlying_type_t<FragmentDynamicState>>(value));
}

constexpr FragmentDynamicState &operator|=(FragmentDynamicState &lhs,
                                           FragmentDynamicState rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr FragmentDynamicState &operator&=(FragmentDynamicState &lhs,
                                           FragmentDynamicState rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr FragmentDynamicState &operator^=(FragmentDynamicState &lhs,
                                           FragmentDynamicState rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(FragmentDynamicState lhs,
           std::underlying_type_t<FragmentDynamicState> rhs) noexcept {
  return static_cast<std::underlying_type_t<FragmentDynamicState>>(lhs) == rhs;
}

[[nodiscard]]
constexpr bool operator==(std::underlying_type_t<FragmentDynamicState> lhs,
                          FragmentDynamicState rhs) noexcept {
  return lhs == static_cast<std::underlying_type_t<FragmentDynamicState>>(rhs);
}

[[nodiscard]]
constexpr bool
operator!=(FragmentDynamicState lhs,
           std::underlying_type_t<FragmentDynamicState> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(std::underlying_type_t<FragmentDynamicState> lhs,
                          FragmentDynamicState rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr VertexDynamicState
operator|(VertexDynamicState lhs,
          VertexDynamicState rhs) noexcept {
  return static_cast<VertexDynamicState>(
      static_cast<std::underlying_type_t<VertexDynamicState>>(lhs) |
      static_cast<std::underlying_type_t<VertexDynamicState>>(rhs));
}

[[nodiscard]]
constexpr VertexDynamicState
operator&(VertexDynamicState lhs,
          VertexDynamicState rhs) noexcept {
  return static_cast<VertexDynamicState>(
      static_cast<std::underlying_type_t<VertexDynamicState>>(lhs) &
      static_cast<std::underlying_type_t<VertexDynamicState>>(rhs));
}

[[nodiscard]]
constexpr VertexDynamicState
operator^(VertexDynamicState lhs,
          VertexDynamicState rhs) noexcept {
  return static_cast<VertexDynamicState>(
      static_cast<std::underlying_type_t<VertexDynamicState>>(lhs) ^
      static_cast<std::underlying_type_t<VertexDynamicState>>(rhs));
}

[[nodiscard]]
constexpr VertexDynamicState
operator~(VertexDynamicState value) noexcept {
  return static_cast<VertexDynamicState>(
      ~static_cast<std::underlying_type_t<VertexDynamicState>>(value));
}

constexpr VertexDynamicState &
operator|=(VertexDynamicState &lhs,
           VertexDynamicState rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr VertexDynamicState &
operator&=(VertexDynamicState &lhs,
           VertexDynamicState rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr VertexDynamicState &
operator^=(VertexDynamicState &lhs,
           VertexDynamicState rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(VertexDynamicState lhs,
           std::underlying_type_t<VertexDynamicState> rhs) noexcept {
  return static_cast<std::underlying_type_t<VertexDynamicState>>(lhs) ==
         rhs;
}

[[nodiscard]]
constexpr bool operator==(std::underlying_type_t<VertexDynamicState> lhs,
                          VertexDynamicState rhs) noexcept {
  return lhs ==
         static_cast<std::underlying_type_t<VertexDynamicState>>(rhs);
}

[[nodiscard]]
constexpr bool
operator!=(VertexDynamicState lhs,
           std::underlying_type_t<VertexDynamicState> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool operator!=(std::underlying_type_t<VertexDynamicState> lhs,
                          VertexDynamicState rhs) noexcept {
  return !(lhs == rhs);
}

} // namespace strobe::gpu::vulkan
