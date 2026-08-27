#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/fence.hpp"
#include "strobe/rhi/objects/timeline_semaphore.hpp"
#include "strobe/rhi/sync/binary_semaphore_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/timeline_semaphore_impl.hpp"
#include "strobe/rhi/types/binary_semaphore_info.hpp"
#include "strobe/rhi/types/fence_info.hpp"
#include "strobe/rhi/types/timeline_semaphore_info.hpp"

namespace strobe::rhi::sync {

struct handle_allocators {
  handle_allocator<BinarySemaphoreImpl> binarySemaphoreAllocator;
  handle_allocator<TimelineSemaphoreImpl> timelineSemaphoreAllocator;
  handle_allocator<FenceImpl> fenceAlloc;
};

BinarySemaphore create_binary_sem(Context context,
                                  const BinarySemaphoreInfo &info,
                                  handle_allocators *alloc);

TimelineSemaphore create_timeline_sem(Context context,
                                      const TimelineSemaphoreInfo &info,
                                      handle_allocators *alloc);

Fence create_fence(Context context, const FenceInfo &info,
                   handle_allocators *alloc);

} // namespace strobe::rhi::sync
