#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/swapchain.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"
namespace strobe::rhi::swap {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : alloc(alloc), swapchainAlloc(alloc) {}

  strobe::rhi::allocator_ref alloc;
  img::handle_allocators *imgAlloc;
  handle_allocator<SwapchainImpl> swapchainAlloc;
};

Swapchain create_swapchain(
    Surface surface, FencePool fencePool, BinarySemaphorePool semPool,
    bool vsync, ImageUsage imageUsage,
    Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
    handle_allocators *allocs);

} // namespace strobe::rhi::swap
