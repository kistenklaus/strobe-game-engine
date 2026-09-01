#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include "strobe/rhi/vulkan/surface.hpp"

namespace strobe::rhi {

Swapchain swap::create_swapchain(
    Context context, FencePool fencePool, BinarySemaphorePool semPool,
    const SwapchainInfo& info,
    Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
    handle_allocators *allocs) {
  ZoneScopedN("swap/create-swapchain");

  vulkan::Surface nativeSurface = vulkan::create_surface(context.ctx(), info.window);

  Surface surface{make_void_handle<SurfaceImpl>(&allocs->surfaceAlloc,
      std::move(context), nativeSurface)};

  return Swapchain{make_void_handle<SwapchainImpl>(
      &allocs->swapchainAlloc, std::move(surface), std::move(fencePool),
      std::move(semPool), info.vsync, info.imageUsage,
      std::move(presentQueueFamilyIndicies), allocs->imgAlloc, allocs->alloc)

  };
}

} // namespace strobe::rhi
