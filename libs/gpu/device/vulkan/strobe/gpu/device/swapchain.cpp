#include "strobe/gpu/device/swapchain.hpp"
#include "strobe/gpu/device/binary_semaphore_impl.hpp"
#include "strobe/gpu/device/fence_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include "strobe/gpu/device/swapchain_impl.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

Swapchain::Swapchain(const Swapchain &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainImpl>(m_handle);
  }
}

Swapchain::Swapchain(Swapchain &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Swapchain &Swapchain::operator=(const Swapchain &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<SwapchainImpl>(o.m_handle);
  }
  unpin_void_handle<SwapchainImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Swapchain &Swapchain::operator=(Swapchain &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Swapchain::~Swapchain() noexcept { unpin_void_handle<SwapchainImpl>(m_handle); }

void Swapchain::resize(uvec2 extent) noexcept {
  ZoneScopedN("Swapchain::resize");
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  impl->desired_extent.store(extent, std::memory_order_relaxed);
}

SwapchainImage Swapchain::acquire(BinarySemaphore signalSemaphore, Fence fence,
                                  uint64_t timeout) {
  ZoneScopedN("Swapchain::acquire");

  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  assert(signalSemaphore);
  // Lazy initial creation.
  if (!impl->generation) {
    impl->recreate();
  }

  while (true) {
    const auto generationHandle = impl->generation.m_handle;
    auto *genImpl = void_handle_ptr<SwapchainGenerationImpl>(generationHandle);
    auto *semImpl =
        void_handle_ptr<BinarySemaphoreImpl>(signalSemaphore.m_handle);
    // The semaphore must not still carry an unconsumed swapchain
    // operation from a previous use.
    assert(semImpl->swapchainGeneration == nullptr);

    vulkan::Fence vkFence{};
    if (fence) {
      auto *fenceImpl = void_handle_ptr<FenceImpl>(fence.m_handle);
      vkFence = fenceImpl->fence;
    }

    VkAcquireNextImageInfoKHR acquireInfo{
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext = nullptr,
        .swapchain = genImpl->swapchain.handle,
        .timeout = timeout,
        .semaphore = semImpl->semaphore.handle,
        .fence = vkFence.handle,
        .deviceMask = 1,
    };
    uint32_t imageIndex = 0;
    VkResult result;
    {
      ZoneScopedN("vkAcquireNextImage2KHR");
      result = vkAcquireNextImage2KHR(impl->context.get()->device(),
                                      &acquireInfo, &imageIndex);
    }

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
      auto &state = genImpl->images[imageIndex];
      state.acquireFence = {};
      assert(!state.acquireSignal);
      state.acquireSignal = signalSemaphore;
      state.acquireFence = fence;
      semImpl->swapchainGeneration = genImpl;

      pin_void_handle<SwapchainGenerationImpl>(generationHandle);
      if (result == VK_SUBOPTIMAL_KHR) {
        impl->generation = {};
      }
      return SwapchainImage{
          generationHandle,
          imageIndex,
      };
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      impl->recreate();
      continue;
    } else if (result == VK_TIMEOUT || result == VK_NOT_READY) {
      return {};
    } else {
      throw std::runtime_error("Failed to acquire swapchain image");
    }
  }
}

} // namespace strobe::gpu
