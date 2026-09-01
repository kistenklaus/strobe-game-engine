#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"

namespace strobe::rhi {

class Swapchain : Object<Swapchain> {
public:
  explicit Swapchain(void *handle) noexcept : Object(handle) {}
  Swapchain() noexcept : Object(nullptr) {}
  Swapchain(const Swapchain &) noexcept;
  Swapchain(Swapchain &&) noexcept;
  Swapchain &operator=(const Swapchain &) noexcept;
  Swapchain &operator=(Swapchain &&) noexcept;
  ~Swapchain() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Swapchain &lhs, const Swapchain &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Swapchain &lhs, const Swapchain &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  SwapchainImage acquire();
};

} // namespace strobe::rhi
