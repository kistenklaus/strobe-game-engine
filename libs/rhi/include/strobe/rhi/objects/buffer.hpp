#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <cassert>
#include <cstdint>

namespace strobe::rhi {

class Buffer : Object<Buffer> {
  friend class Device;
  friend class CommandBuffer;
  friend class MemoryPool;
  friend struct CommandBufferImpl;
  friend struct BlasImpl; // TODO: remove me

public:
  Buffer() noexcept : Object(nullptr) {}
  Buffer(const Buffer &) noexcept;
  Buffer(Buffer &&) noexcept;
  Buffer &operator=(const Buffer &) noexcept;
  Buffer &operator=(Buffer &&) noexcept;
  ~Buffer() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Buffer &lhs, const Buffer &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Buffer &lhs, const Buffer &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
  operator BufferOffset() const noexcept {
    return BufferOffset{.buffer = *this, .offset = 0};
  }

  uint64_t size() const noexcept;

  void *ptr() const;

  void set_name(const char *name) const noexcept;

  explicit Buffer(void *handle) noexcept : Object(handle) { assert(handle); }
};
} // namespace strobe::rhi
