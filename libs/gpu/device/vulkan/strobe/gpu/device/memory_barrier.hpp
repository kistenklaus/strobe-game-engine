#pragma once

#include "strobe/gpu/device/access.hpp"
#include "strobe/gpu/device/pipeline_stage.hpp"

namespace strobe::gpu {

struct MemoryBarrier {
  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
};

} // namespace strobe::gpu
