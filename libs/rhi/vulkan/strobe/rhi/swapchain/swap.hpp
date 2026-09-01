#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/types/swapchain_info.hpp"

namespace strobe::rhi::swap {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc,
      img::handle_allocators* imgAlloc) noexcept
      : alloc(alloc), imgAlloc(imgAlloc), swapchainAlloc(alloc), surfaceAlloc(alloc)  {}

  strobe::rhi::allocator_ref alloc;
  img::handle_allocators *imgAlloc;
  handle_allocator<SwapchainImpl> swapchainAlloc;
  handle_allocator<SurfaceImpl> surfaceAlloc;
};

Swapchain create_swapchain(
    Context surface, FencePool fencePool, BinarySemaphorePool semPool,
    const SwapchainInfo& info,
    Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
    handle_allocators *allocs);

} // namespace strobe::rhi::swap
