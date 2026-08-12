#pragma once

#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/swapchain_impl.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"

namespace strobe::gpu {

struct SwapchainGenerationImpl {
  SwapchainGenerationImpl(Surface surface, vulkan::Swapchain swapchain,
                          span<Image> images, span<ImageView> imageViews,
                          uvec2 extent, Format format)
      : surface(std::move(surface)), images(images), imageViews(imageViews),
        swapchain(swapchain), extent(extent), format(format) {}

  ~SwapchainGenerationImpl() noexcept {
    auto *swapchain_impl = void_handle_ptr<SwapchainImpl>(surface.m_handle);
    auto *device_impl =
        void_handle_ptr<DeviceImpl>(swapchain_impl->device.m_handle);
    vulkan::destroy_swapchain(&device_impl->context, swapchain);
  }

  const Surface surface;
  const SmallVector<Image, 4> images;
  const SmallVector<ImageView, 4> imageViews;
  const vulkan::Swapchain swapchain;
  const uvec2 extent;
  const Format format;
};
} // namespace strobe::gpu
