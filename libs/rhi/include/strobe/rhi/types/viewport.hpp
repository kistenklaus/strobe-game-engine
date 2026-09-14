#pragma once

#include "strobe/core/lina/vec.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Rendering viewport.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} viewport.hpp Viewport
 *
 * Describes the framebuffer region and depth range used during rasterization.
 */
// [Viewport]
struct Viewport {
  vec2 position{0.0f};
  vec2 extent{0.0f};
  float minDepth = 0.0f;
  float maxDepth = 1.0f;
};
// [Viewport]

} // namespace strobe::rhi
