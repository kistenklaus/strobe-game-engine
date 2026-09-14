#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Depth-stencil clear value.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} clear_depth_stencil.hpp ClearDepthStencil
 *
 * Stores depth and stencil values used when clearing a depth-stencil
 * attachment.
 */
// [ClearDepthStencil]
struct ClearDepthStencil {
  float depth;
  uint32_t stencil;
};
// [ClearDepthStencil]

} // namespace strobe::rhi
