#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/buffer.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/queue.hpp"

namespace strobe::rhi {

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

} // namespace strobe::rhi
