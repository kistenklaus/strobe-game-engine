#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include "strobe/rhi/sync/binary_semaphore_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <cassert>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

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
  assert(signalSemaphore &&
         "Swapchain::acquire requires the signalSemaphore to be non null");
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  vulkan::Context *ctx = impl->context.ctx();

  vulkan::Fence acquireFence{};
  if (fence) {
    acquireFence = object_handle_ptr<FenceImpl>(fence)->fence;
  }
  vulkan::BinarySemaphore signal{};
  if (signalSemaphore) {
    signal = object_handle_ptr<BinarySemaphoreImpl>(signalSemaphore)->semaphore;
  }

  // Lazy initial creation.
  if (!impl->generation) {
    impl->recreate();
  }

  while (true) {
    SwapchainGeneration generation = impl->generation;
    auto *gen_impl = object_handle_ptr<SwapchainGenerationImpl>(generation);

    uint32_t imageIndex;
    auto result = vulkan::acquire_next_swapchain_image(
        ctx, gen_impl->swapchain,
        {.timeout = timeout, .signalSemaphore = signal, .fence = acquireFence},
        &imageIndex);

    switch (result) {
    case vulkan::SwapchainAcquireStatus::success: {
      gen_impl->images[imageIndex].acquireSignal = signalSemaphore;
      gen_impl->images[imageIndex].acquireFence = fence;
      return SwapchainImage{make_void_handle<SwapchainImageImpl>(
          impl->swapchainImageHandleAlloc, std::move(generation), imageIndex)};
    }
    case vulkan::SwapchainAcquireStatus::suboptimal: {
      gen_impl->images[imageIndex].acquireSignal = signalSemaphore;
      gen_impl->images[imageIndex].acquireFence = fence;
      impl->generation = {};
      return SwapchainImage{make_void_handle<SwapchainImageImpl>(
          impl->swapchainImageHandleAlloc, std::move(generation), imageIndex)};
    }
    case vulkan::SwapchainAcquireStatus::out_of_date: {
      impl->recreate();
      continue;
    }
    case vulkan::SwapchainAcquireStatus::timeout:
    case vulkan::SwapchainAcquireStatus::not_ready:
      return SwapchainImage{};
    }
  }
}

} // namespace strobe::rhi
