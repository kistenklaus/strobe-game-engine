#include "strobe/rhi/queue/que.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"

namespace strobe::rhi {

Queue que::create_queue(Timeline timeline, GarbageCollector gc,
                        vulkan::Queue queue, QueueFlags flags,
                        que::handle_allocators *alloc) {
  ZoneScopedN("que/create-queue");
  return Queue{make_void_handle<QueueImpl>(
      &alloc->queueAlloc, std::move(timeline), std::move(gc), queue, flags)};
}

} // namespace strobe::rhi
