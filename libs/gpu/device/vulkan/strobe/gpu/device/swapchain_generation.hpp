#pragma once

namespace strobe::gpu {

class SwapchainGeneration {
  friend class Swapchain;
  friend struct SwapchainImpl;
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

private:
  SwapchainGeneration(void* handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
