#pragma once

#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/image.hpp"

namespace strobe::rhi {

class SwapchainGeneration : Object<SwapchainGeneration> {
  friend class Swapchain;
  friend struct SwapchainImpl;
  friend class Queue;

public:
  SwapchainGeneration() noexcept : Object(nullptr) {}
  SwapchainGeneration(const SwapchainGeneration &) noexcept;
  SwapchainGeneration(SwapchainGeneration &&) noexcept;
  SwapchainGeneration &operator=(const SwapchainGeneration &) noexcept;
  SwapchainGeneration &operator=(SwapchainGeneration &&) noexcept;
  ~SwapchainGeneration() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }

  const Image &image(uint32_t index) const noexcept;
  const ImageView &view(uint32_t index) const noexcept;
  const BinarySemaphore &presentReady(uint32_t index) const noexcept;

  explicit SwapchainGeneration(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi
