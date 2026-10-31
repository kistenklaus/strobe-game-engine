#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/object_factory.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include "strobe/rhi/vulkan/surface.hpp"

namespace strobe::rhi {

Swapchain swap::create_swapchain(
    Context context, FencePool fencePool, BinarySemaphorePool semPool,
    ResourceDescriptorHeap rheap, const SwapchainInfo &info,
    Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
    handle_allocators *allocs) {
  ZoneScopedN("swap/create-swapchain");

  vulkan::Surface nativeSurface =
      vulkan::create_surface(context.ctx(), info.window);

  Surface surface = detail::make_object<Surface>(make_void_handle<SurfaceImpl>(
      &allocs->surfaceAlloc, std::move(context), nativeSurface));

  return detail::make_object<Swapchain>(make_void_handle<SwapchainImpl>(
      &allocs->swapchainAlloc, std::move(surface), std::move(fencePool),
      std::move(semPool), std::move(rheap), info.vsync, info.imageUsage, info.srgb,
      std::move(presentQueueFamilyIndicies), allocs->imgAlloc, allocs->alloc));
}

} // namespace strobe::rhi
