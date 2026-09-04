#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/swapchain/surface.hpp"
#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"
#include "strobe/rhi/types/image_usage.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/image_usage_utils.hpp"
#include "strobe/rhi/vulkan/surface.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <atomic>
#include <stdexcept>

namespace strobe::rhi {

struct SwapchainImpl {

  explicit SwapchainImpl(
      Surface surface, FencePool fencePool, BinarySemaphorePool semPool,
      bool vsync, ImageUsage imageUsage,
      Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies,
      img::handle_allocators *imgAlloc,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_surface(std::move(surface)), m_fencePool(std::move(fencePool)),
        m_semPool(std::move(semPool)), m_sync(vsync), m_imageUsage(imageUsage),
        presentQueueFamilyIndicies(std::move(presentQueueFamilyIndicies)),
        m_alloc(alloc), m_imgAlloc(imgAlloc), m_generationAllocator(alloc),
        m_swapchainImageAllocator(alloc) {}
  SwapchainImpl(const SwapchainImpl &) = delete;
  SwapchainImpl(SwapchainImpl &&) = delete;
  ~SwapchainImpl() noexcept = default;

  void resize(uvec2 extent) noexcept {
    m_extent.store(extent, std::memory_order_release);
  }

  bool recreate() {
    ZoneScopedN("swap/recreate");
    const vulkan::SurfaceCapabilities capabilities =
        vulkan::query_surface_capabilities(m_surface.ctx(), m_surface.get());

    // ==== image count =====
    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0) {
      minImageCount = std::min(minImageCount, capabilities.maxImageCount);
    }

    // ==== Select format =====
    auto formats =
        vulkan::query_surface_formats(m_surface.ctx(), m_surface.get());
    if (formats.empty()) {
      throw std::runtime_error("Surface exposes no supported formats");
    }
    VkSurfaceFormatKHR format = formats[0];
    for (const VkSurfaceFormatKHR &candidate : formats) {
      if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB &&
          candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        format = candidate;
        break;
      }
    }
    if (!(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
      for (const VkSurfaceFormatKHR &candidate : formats) {
        if (candidate.format == VK_FORMAT_R8G8B8A8_SRGB &&
            candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
          format = candidate;
          break;
        }
      }
    }

    // ==== Select present mode =====
    auto present_modes =
        vulkan::query_present_modes(m_surface.ctx(), m_surface.get());
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!m_sync) {
      if (std::find(present_modes.begin(), present_modes.end(),
                    VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end()) {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      } else if (std::find(present_modes.begin(), present_modes.end(),
                           VK_PRESENT_MODE_IMMEDIATE_KHR) !=
                 present_modes.end()) {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      }
    }

    // ===== Select extent =====
    uvec2 extent = m_extent.load(std::memory_order_acquire);
    if (capabilities.currentExtent.width != UINT32_MAX) {
      // ingnore user specir
      extent = {capabilities.currentExtent.width,
                capabilities.currentExtent.height};
    } else if (extent.x() != 0 && extent.y() != 0) {
      extent.x() = std::clamp(extent.x(), capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
      extent.y() = std::clamp(extent.y(), capabilities.minImageExtent.height,
                              capabilities.maxImageExtent.height);
    } else {
      return false; // minimized
    }

    // ===== Select alpha composite ======
    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    if (capabilities.supportedCompositeAlpha &
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else if (capabilities.supportedCompositeAlpha &
               VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    } else if (capabilities.supportedCompositeAlpha &
               VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
      compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    } else {
      compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }

    vulkan::Swapchain oldSwapchain;
    if (generation) {
      auto *gen = object_handle_ptr<SwapchainGenerationImpl>(generation);
      oldSwapchain = gen->swapchain;
    }

    vulkan::SwapchainInfo createInfo{
        .surface = m_surface.get(),
        .minImageCount = minImageCount,
        .format = format,
        .extent = extent,
        .usage = to_vk_image_usage(m_imageUsage),
        .queueFamilyIndicies = presentQueueFamilyIndicies,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode,
        .clipped = true,
    };

    vulkan::Swapchain swapchain;

    if (generation) {
      auto *oldGeneration =
          object_handle_ptr<SwapchainGenerationImpl>(generation);

      std::lock_guard lock{oldGeneration->mutex};

      createInfo.oldSwapchain = oldGeneration->swapchain;
      swapchain = vulkan::create_swapchain(m_surface.ctx(), createInfo);
    } else {
      swapchain = vulkan::create_swapchain(m_surface.ctx(), createInfo);
    }

    uint32_t count = vulkan::get_swapchain_images(m_surface.ctx(), swapchain);
    SmallVector<vulkan::Image, 8> nativeImages{count};
    vulkan::get_swapchain_images(m_surface.ctx(), swapchain, nativeImages);

    Vector<SwapchainFrame, strobe::rhi::allocator_ref> frames{
        nativeImages.size(), m_alloc};
    for (uint32_t i = 0; i < frames.size(); ++i) {
      frames[i].image = Image{make_void_handle<ImageImpl>(
          &m_imgAlloc->imageAllocator, m_surface.context(), MemoryAllocation{},
          nativeImages[i], ImageType::image_2d, from_vk_format(format.format),
          uvec3{extent.x(), extent.y(), 1}, 1, 1, SampleCount::x1)};
      frames[i].view =
          img::create_image_view(frames[i].image,
                                 {
                                     .type = ImageViewType::image_2d,
                                     .format = Format::undefined,
                                     .range = {.aspect = ImageAspect::color},
                                 },
                                 m_imgAlloc);
    }

    generation = SwapchainGeneration{make_void_handle<SwapchainGenerationImpl>(
        &m_generationAllocator, m_surface, m_fencePool, m_semPool, swapchain,
        std::move(frames), extent, from_vk_format(format.format),
        &m_swapchainImageAllocator, m_alloc)};
    return true;
  }

private:
  const Surface m_surface{};
  const FencePool m_fencePool{};
  const BinarySemaphorePool m_semPool{};
  const bool m_sync;
  const ImageUsage m_imageUsage;
  const Vector<uint32_t, strobe::rhi::allocator_ref> presentQueueFamilyIndicies;
  strobe::rhi::allocator_ref m_alloc;
  img::handle_allocators *m_imgAlloc;
  handle_allocator<SwapchainGenerationImpl> m_generationAllocator;
  handle_allocator<SwapchainImageImpl> m_swapchainImageAllocator;
  std::atomic<uvec2> m_extent{uvec2(0, 0)};

public:
  SwapchainGeneration generation{};
};

} // namespace strobe::rhi
