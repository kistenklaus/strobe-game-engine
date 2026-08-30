#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
namespace strobe::rhi {

class BinarySemaphorePool : Object<BinarySemaphorePool> {
  public:
    explicit BinarySemaphorePool(void* handle) noexcept : Object(handle) {}
    BinarySemaphorePool() noexcept : Object(nullptr) {}
    BinarySemaphorePool(const BinarySemaphorePool &) noexcept;
    BinarySemaphorePool(BinarySemaphorePool &&) noexcept;
    BinarySemaphorePool &operator=(const BinarySemaphorePool &) noexcept;
    BinarySemaphorePool &operator=(BinarySemaphorePool &&) noexcept;
    ~BinarySemaphorePool() noexcept;
    explicit operator bool() const noexcept {
      return m_handle != nullptr;
    }
    friend bool operator==(const BinarySemaphorePool& lhs, const BinarySemaphorePool& rhs) noexcept {
      return lhs.m_handle == rhs.m_handle;
    }
    friend bool operator!=(const BinarySemaphorePool& lhs, const BinarySemaphorePool& rhs) noexcept {
      return lhs.m_handle != rhs.m_handle;
    }

    BinarySemaphore allocate() noexcept;
};

}
