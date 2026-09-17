#pragma once

#include "strobe/rhi/types/image_aspect.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief image subresource layers
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_subresource_layers.hpp ImageSubresourceLayers
 */
// [ImageSubresourceLayers]
struct ImageSubresourceLayers {
  ImageAspect aspect = ImageAspect::none;
  uint32_t mipLevel = 0;
  uint32_t baseArrayLayer = 0;
  uint32_t layerCount = REMAINING_ARRAY_LAYERS;
};
// [ImageSubresourceLayers]

} // namespace strobe::rhi
