#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/shader_stage.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex shader creation information.
 *
 * Describes the SPIR-V module and the shader stage executed after the vertex
 * shader.
 */
struct VertexShaderInfo {
  /** SPIR-V shader code. */
  span<const uint32_t> spirv = {};

  /** Shader stage following the vertex stage. */
  ShaderStage nextStage = ShaderStage::fragment;
};

} // namespace strobe::rhi
