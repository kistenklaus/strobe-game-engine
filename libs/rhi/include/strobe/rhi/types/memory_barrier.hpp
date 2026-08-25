#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

struct MemoryBarrier {
  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
};

} // namespace strobe::rhi
