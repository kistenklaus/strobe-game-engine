#pragma once

#include "strobe/gpu/vulkan/queue.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"

namespace strobe::gpu {

struct NativeQueue {
  vulkan::Queue queue;
  std::mutex mutex;

  vulkan::TimelineSemaphore timeline;
  std::atomic<uint64_t> nextTimelineValue;

  // counts the amount of Queues which reference this native queue
  std::atomic<uint32_t> users;
};

} // namespace strobe::gpu
