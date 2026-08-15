#pragma once

#include <cstdint>
#include <limits>
namespace strobe::gpu {

class Fence {
  friend class Device;
  friend class Queue;
  friend class Swapchain;
  friend struct SwapchainImpl;

public:
  Fence() noexcept : m_handle(nullptr) {}
  Fence(const Fence &) noexcept;
  Fence(Fence &&) noexcept;
  Fence &operator=(const Fence &) noexcept;
  Fence &operator=(Fence &&) noexcept;
  ~Fence() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  bool wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept;
  void reset() noexcept;

  bool wait_and_reset(
      uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept;
  bool signaled() const noexcept;

private:
  explicit Fence(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
