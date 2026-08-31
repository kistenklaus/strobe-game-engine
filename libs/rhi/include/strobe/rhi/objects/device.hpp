#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class Device : Object<Device> {
public:
  explicit Device(void *handle) noexcept : Object(handle) {}
  Device() noexcept : Object(nullptr) {}
  Device(const Device &) noexcept;
  Device(Device &&) noexcept;
  Device &operator=(const Device &) noexcept;
  Device &operator=(Device &&) noexcept;
  ~Device() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Device &lhs, const Device &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Device &lhs, const Device &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
