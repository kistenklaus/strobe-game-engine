#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/gpu/device/fence.hpp"
#include "strobe/gpu/device/timeline_semaphore.hpp"
namespace strobe::gpu {

struct QueueSubmission {
  uint64_t timelineValue;
  Vector<CommandBuffer, strobe::gpu::allocator> commandBuffers;
  Vector<BinarySemaphore, strobe::gpu::allocator> binarySemaphores;
  Vector<TimelineSemaphore, strobe::gpu::allocator> timelineSemaphores;
  Fence fence;
};

}
