#pragma once

#include "strobe/core/lina/vec.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Rendering viewport.
 *
 * Describes the framebuffer region and depth range used during rasterization.
 */
struct Viewport {
  /** Viewport position in framebuffer coordinates. */
  vec2 position{0.0f};

  /** Viewport width and height. */
  vec2 extent{0.0f};

  /** Minimum viewport depth. */
  float minDepth = 0.0f;

  /** Maximum viewport depth. */
  float maxDepth = 1.0f;
};

} // namespace strobe::rhi
