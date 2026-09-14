#pragma once

#include "strobe/core/lina/vec.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Integer rectangle.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} rect.hpp Rect
 *
 * Describes a two-dimensional rectangular region using an offset and extent.
 */
// [Rect]
struct Rect {
  ivec2 offset = {0, 0};
  uvec2 extent = {0, 0};
};
// [Rect]

} // namespace strobe::rhi
