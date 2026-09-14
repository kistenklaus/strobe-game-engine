#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/shader_stage.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex shader creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} vertex_shader_info.hpp VertexShaderInfo
 *
 * Describes the SPIR-V module and the shader stage executed after the vertex
 * shader.
 */
// [VertexShaderInfo]
struct VertexShaderInfo {
  span<const uint32_t> spirv = {};
  ShaderStage nextStage = ShaderStage::fragment;
};
// [VertexShaderInfo]

} // namespace strobe::rhi
