#pragma once

#include "strobe/rhi/types/buffer_barrier.hpp"
#include "strobe/rhi/types/image_barrier.hpp"
#include "strobe/rhi/types/memory_barrier.hpp"
#include "strobe/core/containers/span.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Pipeline Barrier description.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet pipeline_barrier PipelineBarrier
 *
 * Describes first and second synchronization scopes of a pipepline barrier.
 */
// [PipelineBarrier]
struct PipelineBarrier {
  span<const MemoryBarrier> memoryBarriers;
  span<const BufferBarrier> bufferBarriers;
  span<const ImageBarrier> imageBarriers;
};
// [PipelineBarrier]

} // namespace strobe::rhi
