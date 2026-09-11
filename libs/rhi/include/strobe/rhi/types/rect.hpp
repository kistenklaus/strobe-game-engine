#pragma once

#include "strobe/core/lina/vec.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Integer rectangle.
 *
 * Describes a two-dimensional rectangular region using an offset and extent.
 */
struct Rect {
  /** Offset of the rectangle origin. */
  ivec2 offset = {0, 0};

  /** Width and height of the rectangle. */
  uvec2 extent = {0, 0};
};

} // namespace strobe::rhi
