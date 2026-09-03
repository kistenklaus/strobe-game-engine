#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class BufferDescriptorArray : Object<BufferDescriptorArray> {
public:
  explicit BufferDescriptorArray(void *handle) noexcept : Object(handle) {}
  BufferDescriptorArray() noexcept : Object(nullptr) {}
  BufferDescriptorArray(const BufferDescriptorArray &) noexcept;
  BufferDescriptorArray(BufferDescriptorArray &&) noexcept;
  BufferDescriptorArray &operator=(const BufferDescriptorArray &) noexcept;
  BufferDescriptorArray &operator=(BufferDescriptorArray &&) noexcept;
  ~BufferDescriptorArray() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const BufferDescriptorArray &lhs,
                         const BufferDescriptorArray &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const BufferDescriptorArray &lhs,
                         const BufferDescriptorArray &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
