#pragma once

#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Memory dependency description.
 *
 * Describes source and destination synchronization scopes for a memory barrier.
 */
struct MemoryBarrier {
  /** Source pipeline stages. */
  PipelineStage srcStage = PipelineStage::none;

  /** Source memory access types. */
  Access srcAccess = Access::none;

  /** Destination pipeline stages. */
  PipelineStage dstStage = PipelineStage::none;

  /** Destination memory access types. */
  Access dstAccess = Access::none;
};

} // namespace strobe::rhi
