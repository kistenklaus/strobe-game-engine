#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"

namespace strobe::rhi {

SwapchainImage::SwapchainImage(const SwapchainImage &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(m_handle);
  }
}

SwapchainImage::SwapchainImage(SwapchainImage &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

SwapchainImage &SwapchainImage::operator=(const SwapchainImage &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<SwapchainImageImpl>(o.m_handle);
  }
  unpin_void_handle<SwapchainImageImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

SwapchainImage &SwapchainImage::operator=(SwapchainImage &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainImageImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

SwapchainImage::~SwapchainImage() noexcept {
  unpin_void_handle<SwapchainImageImpl>(m_handle);
}

const Image &SwapchainImage::image() const noexcept {
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  return impl->generation.image(impl->imageIndex);
}

const ImageView &SwapchainImage::view() const noexcept {
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  return impl->generation.view(impl->imageIndex);
}

const BinarySemaphore &SwapchainImage::presentReady() const noexcept {
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  return impl->generation.presentReady(impl->imageIndex);
}

} // namespace strobe::rhi
