#pragma once

#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/objects/image.hpp"
#include <utility>

namespace strobe::rhi {

class SwapchainImage : Object<SwapchainImage> {
  friend class Swapchain;
  friend class Queue;

public:
  SwapchainImage() : Object(nullptr) {}

  SwapchainImage(const SwapchainImage &) noexcept;
  SwapchainImage(SwapchainImage &&) noexcept;
  SwapchainImage &operator=(const SwapchainImage &) noexcept;
  SwapchainImage &operator=(SwapchainImage &&) noexcept;
  ~SwapchainImage() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  const Image &image() const noexcept;
  const ImageView &view() const noexcept;
  const BinarySemaphore &presentReady() const noexcept;

  const uvec2 extent() const noexcept { return image().extent().xy(); }

  explicit SwapchainImage(void *handle) : Object(std::move(handle)) {}
};

} // namespace strobe::rhi
