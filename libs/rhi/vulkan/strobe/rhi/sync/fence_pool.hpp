#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/fence.hpp"

namespace strobe::rhi {

class FencePool : Object<FencePool> {
public:
  explicit FencePool(void *handle) noexcept : Object(handle) {}
  FencePool() noexcept : Object(nullptr) {}
  FencePool(const FencePool &) noexcept;
  FencePool(FencePool &&) noexcept;
  FencePool &operator=(const FencePool &) noexcept;
  FencePool &operator=(FencePool &&) noexcept;
  ~FencePool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const FencePool &lhs, const FencePool &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const FencePool &lhs, const FencePool &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  Fence allocate() noexcept;
};

} // namespace strobe::rhi
