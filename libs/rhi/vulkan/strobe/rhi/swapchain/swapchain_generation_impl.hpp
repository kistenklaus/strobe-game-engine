#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/surface_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_frame.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_present_frame.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/binary_semaphore_pool.hpp"
#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/types/format.hpp"

#include "strobe/rhi/vulkan/swapchain.hpp"

namespace strobe::rhi {

struct SwapchainGenerationImpl {

  explicit SwapchainGenerationImpl(
      Surface surface, FencePool fencePool, BinarySemaphorePool semPool,
      ResourceDescriptorHeap rheap, vulkan::Swapchain swapchain,
      Vector<SwapchainFrame, strobe::rhi::allocator_ref> frames, uvec2 extent,
      Format format,
      handle_allocator_ref<SwapchainImageImpl> swapchainImageHandleAllocator,
      img::handle_allocators *imgAlloc,
      strobe::rhi::allocator_ref alloc) noexcept
      : surface(std::move(surface)), fencePool(std::move(fencePool)),
        semPool(semPool), rheap(std::move(rheap)), swapchain(swapchain),
        frames(std::move(frames)), extent(extent), format(format),
        m_presentFramePool(alloc), m_imageAlloc(swapchainImageHandleAllocator),
        m_imgAlloc(imgAlloc) {}

  SwapchainGenerationImpl(const SwapchainGenerationImpl &) = delete;
  SwapchainGenerationImpl(SwapchainGenerationImpl &&) = delete;

  ~SwapchainGenerationImpl() noexcept {
    vulkan::destroy_swapchain(surface.ctx(), swapchain);
  }

  handle_allocator_ref<SwapchainImageImpl>
  get_swapchain_image_handle_allocator() const noexcept {
    return m_imageAlloc;
  }

  swapchain_present_frame_pool_ref get_present_frame_allocator() noexcept {
    return &m_presentFramePool;
  }

  const ImageView &lazy_image_view(uint32_t index) noexcept {
    if (!frames[index].lazyView) {
      frames[index].lazyView =
          img::create_image_view(frames[index].image,
                                 {
                                     .type = ImageViewType::image_2d,
                                     .range = {.aspect = ImageAspect::color},
                                 },
                                 m_imgAlloc);
    }
    return frames[index].lazyView;
  }

  const ResourceDescriptor &
  lazy_sampled_image_descriptor(uint32_t index) noexcept {
    if (!frames[index].lazySampledImageDescriptor) {
      frames[index].lazySampledImageDescriptor =
          rheap.create_descriptor(SampledImageDescriptorInfo{
              .image = frames[index].image,
              .layout = strobe::rhi::ImageLayout::general,
              .viewType = ImageViewType::image_2d,
              .subresource{.aspect = ImageAspect::color}});
    }
    return frames[index].lazySampledImageDescriptor;
  }

  const ResourceDescriptor &
  lazy_storage_image_descriptor(uint32_t index) noexcept {
    if (!frames[index].lazyStorageImageDescriptor) {
      frames[index].lazyStorageImageDescriptor =
          rheap.create_descriptor(StorageImageDescriptorInfo{
              .image = frames[index].image,
              .layout = strobe::rhi::ImageLayout::general,
              .viewType = ImageViewType::image_2d,
              .subresource{.aspect = ImageAspect::color}});
    }
    return frames[index].lazyStorageImageDescriptor;
  }

  Surface surface;
  FencePool fencePool;
  BinarySemaphorePool semPool;
  ResourceDescriptorHeap rheap;
  vulkan::Swapchain swapchain;
  Vector<SwapchainFrame, strobe::rhi::allocator_ref> frames;
  uvec2 extent;
  Format format;
  bool suboptimal = false;
  bool dead = false;

  std::atomic<uint64_t> debugCounter{0};

  std::mutex mutex;

private:
  swapchain_present_frame_pool m_presentFramePool;
  handle_allocator_ref<SwapchainImageImpl> m_imageAlloc;
  img::handle_allocators *m_imgAlloc;
};

} // namespace strobe::rhi
