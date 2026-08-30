#include "strobe/rhi/sync/sync.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/sync/timeline_semaphore_impl.hpp"
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

TimelineSemaphore create(Context context, const TimelineSemaphoreInfo &info,
                         handle_allocators *alloc) {
  const vulkan::TimelineSemaphore sem = vulkan::create_timeline_semaphore(
      context.ctx(), {.initalValue = info.initialValue});
  return TimelineSemaphore{make_void_handle<TimelineSemaphoreImpl>(
      &alloc->timelineSemaphoreAllocator, std::move(context), sem)};
}

} // namespace strobe::rhi::sync
