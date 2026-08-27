#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

struct BinarySemaphore : Object<BinarySemaphore> {
  friend class Device;
  friend class Queue;
  friend class Swapchain;
  friend struct SwapchainImpl;

public:
  BinarySemaphore() noexcept : Object(nullptr) {}
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

  explicit BinarySemaphore(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi
