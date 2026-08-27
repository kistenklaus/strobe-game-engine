#include "strobe/rhi/sync/sync.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_impl.hpp"
#include "strobe/rhi/sync/timeline_semaphore_impl.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::sync {

BinarySemaphore
create_binary_sem(Context context,
                  [[maybe_unused]] const BinarySemaphoreInfo &info,
                  handle_allocators* alloc) {
  const vulkan::BinarySemaphore sem =
      vulkan::create_binary_semaphore(context.ctx(), {});
  return BinarySemaphore{
      make_void_handle<BinarySemaphoreImpl>(&alloc->binarySemaphoreAllocator, std::move(context), sem)};
}

TimelineSemaphore create(Context context, const TimelineSemaphoreInfo &info,
                         handle_allocators* alloc) {
  const vulkan::TimelineSemaphore sem = vulkan::create_timeline_semaphore(
      context.ctx(), {.initalValue = info.initialValue});
  return TimelineSemaphore{
      make_void_handle<TimelineSemaphoreImpl>(&alloc->timelineSemaphoreAllocator, std::move(context), sem)};
}

Fence create_fence(Context context, const FenceInfo &info,
                   handle_allocators* alloc) {
  const vulkan::Fence fence = vulkan::create_fence(
      context.ctx(),
      {.flags = info.signaled ? VkFenceCreateFlags(VK_FENCE_CREATE_SIGNALED_BIT)
                              : VkFenceCreateFlags(0)});
  return Fence{make_void_handle<FenceImpl>(&alloc->fenceAlloc, std::move(context), fence)};
}

} // namespace strobe::rhi::sync
