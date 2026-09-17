#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/types/image_subresource_layers.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief image range
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_range.hpp ImageRange
 */
// [ImageRange]
struct ImageRange {
  Image image;
  ImageSubresourceLayers subresource{};
  ivec3 offset{};
  uvec3 extent{};
};
// [ImageRange]

} // namespace strobe::rhi
