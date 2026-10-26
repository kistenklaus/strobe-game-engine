#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer Barrier description.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet buffer_barrier.hpp BufferBarrier
 *
 * Describes first and second synchronization scopes of a buffer barrier.
 */
// [BufferBarrier]
struct BufferBarrier {
  Buffer buffer;
  PipelineStage srcStage = PipelineStage::none;
  Access srcAccess = Access::none;
  PipelineStage dstStage = PipelineStage::none;
  Access dstAccess = Access::none;
};
// [BufferBarrier]

} // namespace strobe::rhi
