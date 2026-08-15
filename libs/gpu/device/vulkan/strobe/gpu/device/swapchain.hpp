#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/fence.hpp"
#include "strobe/gpu/device/image_usage.hpp"
#include "strobe/gpu/device/swapchain_image.hpp"
#include <cstdint>
#include <limits>
namespace strobe::gpu {

struct SwapchainCreateInfo {
  uvec2 extent = {0, 0};
  ImageUsage imageUsage = ImageUsage::color_attachment;
  bool vsync = false;
  bool clipped = true;
};

class Swapchain {
  friend struct SwapchainGenerationImpl;
  friend class Device;

public:
  Swapchain() noexcept : m_handle(nullptr) {}
  Swapchain(const Swapchain &) noexcept;
  Swapchain(Swapchain &&) noexcept;
  Swapchain &operator=(const Swapchain &) noexcept;
  Swapchain &operator=(Swapchain &&) noexcept;
  ~Swapchain() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  // thread-safe!
  void resize(uvec2 extent) noexcept;
  SwapchainImage
  acquire(BinarySemaphore signal, Fence fence = {},
          uint64_t timeout = std::numeric_limits<uint64_t>::max());

private:
  explicit Swapchain(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
