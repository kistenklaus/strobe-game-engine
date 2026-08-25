#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/memory_barrier.hpp"

namespace strobe::rhi {

struct Barrier {
  span<const MemoryBarrier> memoryBarriers = {};
  // span<const BufferMemoryBarrier> bufferBarriers = {};
  // span<const ImageMemoryBarrier> imageBarriers = {};
};

} // namespace strobe::rhi
