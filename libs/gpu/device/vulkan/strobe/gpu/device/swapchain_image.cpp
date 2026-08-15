#include "strobe/gpu/device/swapchain_image.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include <limits>

namespace strobe::gpu {

SwapchainImage::SwapchainImage(const SwapchainImage &o) noexcept
    : m_handle(o.m_handle), m_index(o.m_index) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(m_handle);
  }
}

SwapchainImage::SwapchainImage(SwapchainImage &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)),
      m_index(std::exchange(o.m_index, std::numeric_limits<uint32_t>::max())) {}

SwapchainImage &SwapchainImage::operator=(const SwapchainImage &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(o.m_handle);
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = o.m_handle;
  m_index = o.m_index;
  return *this;
}

SwapchainImage &SwapchainImage::operator=(SwapchainImage &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  m_index = std::exchange(o.m_index, std::numeric_limits<uint32_t>::max());
  return *this;
}

SwapchainImage::~SwapchainImage() noexcept {
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
}

const Image &SwapchainImage::image() const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->images[m_index].image;
}
const BinarySemaphore &SwapchainImage::presentReady() const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->images[m_index].presentReady;
}

} // namespace strobe::gpu
