#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Depth-stencil clear value.
 *
 * Stores depth and stencil values used when clearing a depth-stencil
 * attachment.
 */
struct ClearDepthStencil {
  /** Depth clear value. */
  float depth;

  /** Stencil clear value. */
  uint32_t stencil;
};

} // namespace strobe::rhi
