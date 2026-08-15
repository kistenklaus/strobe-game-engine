#pragma once

namespace strobe::gpu {

struct BinarySemaphore {
  friend class Device;
  friend class Queue;
  friend class Swapchain;
  friend struct SwapchainImpl;

public:
  BinarySemaphore() noexcept : m_handle(nullptr) {}
  BinarySemaphore(const BinarySemaphore &) noexcept;
  BinarySemaphore(BinarySemaphore &&) noexcept;
  BinarySemaphore &operator=(const BinarySemaphore &) noexcept;
  BinarySemaphore &operator=(BinarySemaphore &&) noexcept;
  ~BinarySemaphore() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  void set_name(const char *name) noexcept;

  friend bool operator==(const BinarySemaphore &lhs,
                         const BinarySemaphore &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const BinarySemaphore &lhs,
                         const BinarySemaphore &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

private:
  explicit BinarySemaphore(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
