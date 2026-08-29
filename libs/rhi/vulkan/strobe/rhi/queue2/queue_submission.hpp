#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/objects/timeline_semaphore.hpp"

namespace strobe::rhi {

struct QueueSubmission {
  uint64_t timelineValue;
  Vector<CommandBuffer, strobe::rhi::allocator> commandBuffers;
  Vector<BinarySemaphore, strobe::rhi::allocator> binarySemaphores;
  Vector<TimelineSemaphore, strobe::rhi::allocator> timelineSemaphores;
  Fence fence;
};

} // namespace strobe::rhi
