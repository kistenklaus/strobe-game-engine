#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/shader_stage.hpp"
#include <cstdint>
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief geometry shader create info.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} geometry_shader_info.hpp GeometryShaderInfo
 */
// [GeometryShaderInfo]
struct GeometryShaderInfo {
  span<const uint32_t> spirv = {};
  ShaderStage nextStage = ShaderStage::fragment;
};
// [GeometryShaderInfo]

} // namespace strobe::rhi
