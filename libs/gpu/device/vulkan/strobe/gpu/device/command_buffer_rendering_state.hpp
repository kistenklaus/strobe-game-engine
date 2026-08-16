#pragma once

#include "strobe/gpu/device/command_buffer_rendering_cmd.hpp"

#include <cstdint>
#include <type_traits>

namespace strobe::gpu {

enum class CommandBufferRenderingState : uint64_t {
  none = 0,

  rasterizer_discard_enable =
      1ull << static_cast<uint8_t>(
          CommandBufferRenderingCmd::rasterizer_discard_enable),

  primitive_topology = 1ull << static_cast<uint8_t>(
                           CommandBufferRenderingCmd::primitive_topology),

  primitive_restart_enable =
      1ull << static_cast<uint8_t>(
          CommandBufferRenderingCmd::primitive_restart_enable),

  cull_mode =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::cull_mode),

  front_face =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::front_face),

  depth_bias_enable = 1ull << static_cast<uint8_t>(
                          CommandBufferRenderingCmd::depth_bias_enable),

  depth_test_enable = 1ull << static_cast<uint8_t>(
                          CommandBufferRenderingCmd::depth_test_enable),

  depth_write_enable = 1ull << static_cast<uint8_t>(
                           CommandBufferRenderingCmd::depth_write_enable),

  depth_compare_op =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::depth_compare_op),

  depth_bounds_test_enable =
      1ull << static_cast<uint8_t>(
          CommandBufferRenderingCmd::depth_bounds_test_enable),

  depth_bounds =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::depth_bounds),

  stencil_test_enable = 1ull << static_cast<uint8_t>(
                            CommandBufferRenderingCmd::stencil_test_enable),

  stencil_compare_mask = 1ull << static_cast<uint8_t>(
                             CommandBufferRenderingCmd::stencil_compare_mask),

  vertex_input =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::vertex_input),

  rasterization_samples = 1ull << static_cast<uint8_t>(
                              CommandBufferRenderingCmd::rasterization_samples),

  sample_mask =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::sample_mask),

  alpha_to_coverage_enable =
      1ull << static_cast<uint8_t>(
          CommandBufferRenderingCmd::alpha_to_coverage_enable),

  polygon_mode =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::polygon_mode),

  depth_clamp_enable = 1ull << static_cast<uint8_t>(
                           CommandBufferRenderingCmd::depth_clamp_enable),

  logic_op_enable =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::logic_op_enable),

  color_blend_enable = 1ull << static_cast<uint8_t>(
                           CommandBufferRenderingCmd::color_blend_enable),

  color_write_mask =
      1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::color_write_mask),

  alpha_to_one_enable = 1ull << static_cast<uint8_t>(
                            CommandBufferRenderingCmd::alpha_to_one_enable),

  all = (1ull << static_cast<uint8_t>(CommandBufferRenderingCmd::count)) - 1ull,

  post_rasterization_requirements =
      cull_mode | front_face | polygon_mode | depth_bias_enable |
      depth_test_enable | depth_write_enable | depth_bounds_test_enable |
      stencil_test_enable | rasterization_samples | sample_mask |
      alpha_to_coverage_enable | color_blend_enable | color_write_mask,

  graphics_pipeline_requirements = rasterizer_discard_enable |
                                   depth_clamp_enable |
                                   post_rasterization_requirements,

  vertex_shader_requirements =
      vertex_input | primitive_topology | primitive_restart_enable,

  fragment_shader_requirements = color_blend_enable | color_write_mask,
};

[[nodiscard]]
constexpr CommandBufferRenderingState
operator|(CommandBufferRenderingState lhs,
          CommandBufferRenderingState rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) |
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(rhs));
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator&(CommandBufferRenderingState lhs,
          CommandBufferRenderingState rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) &
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(rhs));
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator^(CommandBufferRenderingState lhs,
          CommandBufferRenderingState rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) ^
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(rhs));
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator~(CommandBufferRenderingState value) noexcept {
  return static_cast<CommandBufferRenderingState>(
      ~static_cast<std::underlying_type_t<CommandBufferRenderingState>>(value));
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator|(CommandBufferRenderingState lhs,
          std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) |
      rhs);
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator|(std::underlying_type_t<CommandBufferRenderingState> lhs,
          CommandBufferRenderingState rhs) noexcept {
  return rhs | lhs;
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator&(CommandBufferRenderingState lhs,
          std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) &
      rhs);
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator&(std::underlying_type_t<CommandBufferRenderingState> lhs,
          CommandBufferRenderingState rhs) noexcept {
  return rhs & lhs;
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator^(CommandBufferRenderingState lhs,
          std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  return static_cast<CommandBufferRenderingState>(
      static_cast<std::underlying_type_t<CommandBufferRenderingState>>(lhs) ^
      rhs);
}

[[nodiscard]]
constexpr CommandBufferRenderingState
operator^(std::underlying_type_t<CommandBufferRenderingState> lhs,
          CommandBufferRenderingState rhs) noexcept {
  return rhs ^ lhs;
}

constexpr CommandBufferRenderingState &
operator|=(CommandBufferRenderingState &lhs,
           CommandBufferRenderingState rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CommandBufferRenderingState &
operator&=(CommandBufferRenderingState &lhs,
           CommandBufferRenderingState rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr CommandBufferRenderingState &
operator^=(CommandBufferRenderingState &lhs,
           CommandBufferRenderingState rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

constexpr CommandBufferRenderingState &
operator|=(CommandBufferRenderingState &lhs,
           std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  lhs = lhs | rhs;
  return lhs;
}

constexpr CommandBufferRenderingState &
operator&=(CommandBufferRenderingState &lhs,
           std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  lhs = lhs & rhs;
  return lhs;
}

constexpr CommandBufferRenderingState &
operator^=(CommandBufferRenderingState &lhs,
           std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  lhs = lhs ^ rhs;
  return lhs;
}

[[nodiscard]]
constexpr bool
operator==(CommandBufferRenderingState lhs,
           std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  return static_cast<std::underlying_type_t<CommandBufferRenderingState>>(
             lhs) == rhs;
}

[[nodiscard]]
constexpr bool
operator==(std::underlying_type_t<CommandBufferRenderingState> lhs,
           CommandBufferRenderingState rhs) noexcept {
  return rhs == lhs;
}

[[nodiscard]]
constexpr bool
operator!=(CommandBufferRenderingState lhs,
           std::underlying_type_t<CommandBufferRenderingState> rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]]
constexpr bool
operator!=(std::underlying_type_t<CommandBufferRenderingState> lhs,
           CommandBufferRenderingState rhs) noexcept {
  return !(rhs == lhs);
}

} // namespace strobe::gpu
