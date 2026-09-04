#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"

namespace strobe::rhi {

Swapchain::Swapchain(const Swapchain &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainImpl>(m_handle);
  }
}

Swapchain::Swapchain(Swapchain &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

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

SwapchainImage Swapchain::acquire() {
  ZoneScopedN("Swapchain::acquire");
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  if (!impl->generation && !impl->recreate()) {
    return {};
  }
  assert(impl->generation);
  // if (impl->generation.suboptimal()) {
  //   impl->recreate();
  // }
  while (true) {
    SwapchainImage image = impl->generation.acquire();
    if (image) {
      return image;
    }
    if (!impl->recreate()) {
      return {}; // minimized
    }
  }
}

} // namespace strobe::rhi
