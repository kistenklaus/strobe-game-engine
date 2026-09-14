#pragma once

#include "strobe/core/containers/span.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Fragment shader creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} fragment_shader_info.hpp FragmentShaderInfo
 *
 * Describes the SPIR-V module used to create a fragment shader.
 */
// [FragmentShaderInfo]
struct FragmentShaderInfo {
  span<const uint32_t> spirv = {};
};
// [FragmentShaderInfo]

} // namespace strobe::rhi
