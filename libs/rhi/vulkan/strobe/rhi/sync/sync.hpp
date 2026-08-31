#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/fence_pool_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/sync/timepoint.hpp"

namespace strobe::rhi::sync {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), binaryPoolAlloc(alloc), fencePoolAlloc(alloc),
        fenceAlloc(alloc), timelineAlloc(alloc)  {}
  strobe::rhi::allocator_ref alloc;
  handle_allocator<BinarySemaphorePoolImpl> binaryPoolAlloc;
  handle_allocator<FencePoolImpl> fencePoolAlloc;
  handle_allocator<FenceImpl> fenceAlloc;
  handle_allocator<TimelineImpl> timelineAlloc;
};

BinarySemaphorePool create_binary_pool(Context context,
                                       handle_allocators *alloc);

FencePool create_fence_pool(Context context, handle_allocators *alloc);

Timeline create_timeline(Context context, handle_allocators *alloc);

} // namespace strobe::rhi::sync
