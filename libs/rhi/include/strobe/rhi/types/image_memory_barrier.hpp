#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/image.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/queue.hpp"

namespace strobe::rhi {

struct ImageMemoryBarrier {
  Image image = {};
  ImageSubresourceRange range = {};

  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  ImageLayout srcLayout = ImageLayout::undefined;
  Queue srcQueue = {};
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
  ImageLayout dstLayout = ImageLayout::undefined;
  Queue dstQueue = {};
};

} // namespace strobe::rhi
