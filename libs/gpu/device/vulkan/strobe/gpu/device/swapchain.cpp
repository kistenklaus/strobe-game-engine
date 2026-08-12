#include "strobe/gpu/device/swapchain.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include "strobe/gpu/device/swapchain_impl.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"
#include <utility>

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
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  impl->desired_extent.store(extent, std::memory_order_relaxed);
}

SwapchainImage Swapchain::acquire(const SwapchainAcquireInfo &info) {
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);

  // Lazy initial creation.
  if (!impl->generation) {
    impl->recreate();
  }
  while (true) {
    auto *gen_impl =
        void_handle_ptr<SwapchainGenerationImpl>(impl->generation.m_handle);

    auto result = vulkan::acquire_next_swapchain_image(
        &void_handle_ptr<DeviceImpl>(impl->device.m_handle)->context,
        gen_impl->swapchain,
        vulkan::SwapchainAcquireInfo{
            .timeout = info.timeout,
        });

    switch (result.status) {
    case vulkan::SwapchainAcquireStatus::success:
      return SwapchainImage{
          impl->generation,
          result.imageIndex,
      };
    case vulkan::SwapchainAcquireStatus::suboptimal:
    case vulkan::SwapchainAcquireStatus::out_of_date: {
      impl->recreate();
      continue;
    }
    case vulkan::SwapchainAcquireStatus::timeout:
    case vulkan::SwapchainAcquireStatus::not_ready:
      return {};
    default:
      throw std::runtime_error("Failed to acquire swapchain image");
    }
  }
}

} // namespace strobe::gpu
