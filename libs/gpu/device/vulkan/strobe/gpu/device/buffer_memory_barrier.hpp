#pragma once

#include "strobe/gpu/device/access.hpp"
#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/pipeline_stage.hpp"
#include "strobe/gpu/device/queue.hpp"

namespace strobe::gpu {

struct BufferMemoryBarrier {
  Buffer buffer;

  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  Queue srcQueue = {};
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
  Queue dstQueue = {};

  uint64_t offset = 0;
  uint64_t size = 0; // <- 0 means buffer.size();
};

} // namespace strobe::gpu
