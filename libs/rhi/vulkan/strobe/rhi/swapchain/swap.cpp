#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"

namespace strobe::rhi {

Swapchain swap::create_swapchain(
    Surface surface, FencePool fencePool, BinarySemaphorePool semPool,
    bool vsync, ImageUsage imageUsage,
    Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
    handle_allocators *allocs) {

  return Swapchain{make_void_handle<SwapchainImpl>(
      &allocs->swapchainAlloc, std::move(surface), std::move(fencePool),
      std::move(semPool), vsync, imageUsage,
      std::move(presentQueueFamilyIndicies), allocs->imgAlloc, allocs->alloc)

  };
}

} // namespace strobe::rhi
