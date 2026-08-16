#pragma once

#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/image.hpp"
#include <cstdint>
#include <limits>
#include <utility>

namespace strobe::gpu {

class SwapchainImage {
  friend class Swapchain;
  friend class Queue;

public:
  SwapchainImage()
      : m_handle(nullptr), m_index(std::numeric_limits<uint32_t>::max()) {}

  SwapchainImage(const SwapchainImage &) noexcept;
  SwapchainImage(SwapchainImage &&) noexcept;
  SwapchainImage &operator=(const SwapchainImage &) noexcept;
  SwapchainImage &operator=(SwapchainImage &&) noexcept;
  ~SwapchainImage() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  const Image &image() const noexcept;
  const ImageView &view() const noexcept;
  const BinarySemaphore &presentReady() const noexcept;

private:
  explicit SwapchainImage(void *handle, uint32_t index)
      : m_handle(std::move(handle)), m_index(index) {}

  void *m_handle;
  uint32_t m_index;
};

} // namespace strobe::gpu
