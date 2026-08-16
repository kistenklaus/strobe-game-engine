#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class CommandBufferRenderingCmd : uint8_t {
  rasterizer_discard_enable,

  primitive_topology,
  primitive_restart_enable,

  cull_mode,
  front_face,

  depth_bias_enable,
  depth_test_enable,
  depth_write_enable,
  depth_compare_op,
  depth_bounds_test_enable,
  depth_bounds,

  stencil_test_enable,
  stencil_compare_mask,

  vertex_input,

  rasterization_samples,
  sample_mask,
  alpha_to_coverage_enable,

  polygon_mode,
  depth_clamp_enable,

  logic_op_enable,

  color_blend_enable,
  color_write_mask,

  alpha_to_one_enable,

  count,
};

} // namespace strobe::gpu
