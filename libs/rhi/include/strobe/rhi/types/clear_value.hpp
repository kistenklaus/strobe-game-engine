#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/types/clear_depth_stencil.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment clear value.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} clear_value.hpp ClearValue
 *
 * Stores a color or depth-stencil value used when clearing an attachment.
 */
// [ClearValue]
union ClearValue {
  vec4 float4 = {};
  uvec4 unsigned4;
  ClearDepthStencil depthStencil;
};
// [ClearValue]

} // namespace strobe::rhi
