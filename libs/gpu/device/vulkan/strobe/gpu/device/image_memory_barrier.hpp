#pragma once

#include "strobe/gpu/device/access.hpp"
#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/image_layout.hpp"
#include "strobe/gpu/device/image_subresource_range.hpp"
#include "strobe/gpu/device/pipeline_stage.hpp"
#include "strobe/gpu/device/queue.hpp"

namespace strobe::gpu {

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

} // namespace strobe::gpu
