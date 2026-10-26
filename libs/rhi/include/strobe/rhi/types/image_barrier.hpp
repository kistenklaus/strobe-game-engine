#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image Barrier description
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet image_barrier.hpp ImageBarrier
 *
 * Describes first and second synchronization scopes of a image barrier.
 */
// [ImageBarrier]
struct ImageBarrier {
  Image image;
  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  ImageLayout srcLayout = ImageLayout::undefined;
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
  ImageLayout dstLayout = ImageLayout::undefined;
  ImageSubresourceRange subresource = {};
};
// [ImageBarrier]

} // namespace strobe::rhi
