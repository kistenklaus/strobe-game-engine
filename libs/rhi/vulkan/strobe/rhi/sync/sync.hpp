#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/timeline_semaphore.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/sync/timeline_semaphore_impl.hpp"
#include "strobe/rhi/types/timeline_semaphore_info.hpp"

namespace strobe::rhi::sync {

struct handle_allocators {
  handle_allocator<BinarySemaphorePoolImpl> binaryPoolAlloc;
  handle_allocator<TimelineSemaphoreImpl> timelineSemaphoreAllocator;
  handle_allocator<FencePoolImpl> fencePoolAlloc;
  handle_allocator<FenceImpl> fenceAlloc;
  strobe::rhi::allocator_ref alloc;
};

BinarySemaphorePool create_binary_pool(Context context,
                                       handle_allocators *alloc);

FencePool create_fence_pool(Context context, handle_allocators *alloc);

[[deprecated]]
TimelineSemaphore create_timeline_sem(Context context,
                                      const TimelineSemaphoreInfo &info,
                                      handle_allocators *alloc);

} // namespace strobe::rhi::sync
