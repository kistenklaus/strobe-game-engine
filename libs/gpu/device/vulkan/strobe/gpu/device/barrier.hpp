#pragma once

#include "strobe/gpu/device/buffer_memory_barrier.hpp"
#include "strobe/gpu/device/image_memory_barrier.hpp"
#include "strobe/gpu/device/memory_barrier.hpp"

namespace strobe::gpu {

struct Barrier {
  span<const MemoryBarrier> memoryBarriers = {};
  span<const BufferMemoryBarrier> bufferBarriers = {};
  span<const ImageMemoryBarrier> imageBarriers = {};
};

} // namespace strobe::gpu
