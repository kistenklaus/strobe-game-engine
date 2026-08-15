#pragma once

#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/image.hpp"
namespace strobe::gpu {

class SwapchainGeneration {
  friend class Swapchain;
  friend struct SwapchainImpl;
  friend class Queue;
public:
  SwapchainGeneration() noexcept : m_handle(nullptr) {}
  SwapchainGeneration(const SwapchainGeneration &) noexcept;
  SwapchainGeneration(SwapchainGeneration &&) noexcept;
  SwapchainGeneration &operator=(const SwapchainGeneration &) noexcept;
  SwapchainGeneration &operator=(SwapchainGeneration &&) noexcept;
  ~SwapchainGeneration() noexcept;

  explicit operator bool() const noexcept {
    return m_handle != nullptr;
  }

  const Image &image(uint32_t index) const noexcept;
  const BinarySemaphore &presentReady(uint32_t index) const noexcept;

private:
  SwapchainGeneration(void* handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
