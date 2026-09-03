#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class BufferDescriptor : Object<BufferDescriptor> {
public:
  explicit BufferDescriptor(void *handle) noexcept : Object(handle) {}
  BufferDescriptor() noexcept : Object(nullptr) {}
  BufferDescriptor(const BufferDescriptor &) noexcept;
  BufferDescriptor(BufferDescriptor &&) noexcept;
  BufferDescriptor &operator=(const BufferDescriptor &) noexcept;
  BufferDescriptor &operator=(BufferDescriptor &&) noexcept;
  ~BufferDescriptor() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const BufferDescriptor &lhs,
                         const BufferDescriptor &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const BufferDescriptor &lhs,
                         const BufferDescriptor &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
