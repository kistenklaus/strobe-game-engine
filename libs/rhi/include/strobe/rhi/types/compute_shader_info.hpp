#pragma once

#include "strobe/core/containers/span.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Compute shader creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} compute_shader_info.hpp ComputeShaderInfo
 *
 * Describes the SPIR-V module used to create a compute shader.
 */
// [ComputeShaderInfo]
struct ComputeShaderInfo {
  span<const uint32_t> spirv = {};
};
// [ComputeShaderInfo]

} // namespace strobe::rhi
