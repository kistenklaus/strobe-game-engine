#pragma once

#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

// NOTE: semaphores are not objects, custom shared ptr implementation with
// incompatible layout!!!
struct BinarySemaphore {
  friend class BinarySemaphorePool;

public:
  BinarySemaphore() noexcept : m_handle(nullptr) {}
  BinarySemaphore(const BinarySemaphore &) noexcept;
  BinarySemaphore(BinarySemaphore &&) noexcept;
  BinarySemaphore &operator=(const BinarySemaphore &) noexcept;
  BinarySemaphore &operator=(BinarySemaphore &&) noexcept;
  ~BinarySemaphore() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  friend bool operator==(const BinarySemaphore &lhs,
                         const BinarySemaphore &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const BinarySemaphore &lhs,
                         const BinarySemaphore &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  vulkan::BinarySemaphore signal() const noexcept;
  vulkan::BinarySemaphore wait() const noexcept;

private:
  explicit BinarySemaphore(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::rhi
