#include "strobe/gpu/device/swapchain_generation.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"

namespace strobe::gpu {

SwapchainGeneration::SwapchainGeneration(const SwapchainGeneration &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(m_handle);
  }
}

SwapchainGeneration::SwapchainGeneration(SwapchainGeneration &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

SwapchainGeneration &
SwapchainGeneration::operator=(const SwapchainGeneration &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(o.m_handle);
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

SwapchainGeneration &
SwapchainGeneration::operator=(SwapchainGeneration &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

SwapchainGeneration::~SwapchainGeneration() noexcept {
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
}

const Image &SwapchainGeneration::image(uint32_t index) const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->images[index].image;
}

const BinarySemaphore &
SwapchainGeneration::presentReady(uint32_t index) const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->images[index].presentReady;
}

} // namespace strobe::gpu
