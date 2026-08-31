#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/queue/queue.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"
namespace strobe::rhi::que {

struct handle_allocators {

  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), queueAlloc(alloc) {}

  strobe::rhi::allocator_ref alloc;
  handle_allocator<QueueImpl> queueAlloc;
};

Queue create_queue(Timeline timeline, GarbageCollector gc, vulkan::Queue queue,
                   QueueFlags flags, que::handle_allocators *alloc);

} // namespace strobe::rhi::que
