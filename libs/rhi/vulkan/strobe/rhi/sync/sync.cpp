#include "strobe/rhi/sync/sync.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::sync {

BinarySemaphorePool create_binary_pool(Context context,
                                       handle_allocators *alloc) {

  return BinarySemaphorePool{make_void_handle<BinarySemaphorePoolImpl>(
      &alloc->binaryPoolAlloc, std::move(context), alloc->alloc)};
}

FencePool create_fence_pool(Context context, handle_allocators *alloc) {
  return FencePool{make_void_handle<FencePoolImpl>(
      &alloc->fencePoolAlloc, std::move(context), &alloc->fenceAlloc,
      alloc->alloc)};
}

Timeline create_timeline(Context context, handle_allocators *alloc) {

  const vulkan::TimelineSemaphore sem =
      vulkan::create_timeline_semaphore(context.ctx());
  return Timeline{
      make_void_handle<TimelineImpl>(&alloc->timelineAlloc, std::move(context),
                                     sem),
  };
}

} // namespace strobe::rhi::sync
