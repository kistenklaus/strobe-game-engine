#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Memory dependency description.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet memory_barrier.hpp MemoryBarrier
 *
 * Describes source and destination synchronization scopes for a memory barrier.
 */
// [MemoryBarrier]
struct MemoryBarrier {
  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
};
// [MemoryBarrier]

} // namespace strobe::rhi
