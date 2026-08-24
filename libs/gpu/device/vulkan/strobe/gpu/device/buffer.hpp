#pragma once

#include "strobe/gpu/device/buffer_usage.hpp"
#include "strobe/gpu/device/memory_usage.hpp"
#include <cassert>
#include <cstdint>
namespace strobe::gpu {

struct BufferInfo {
  uint64_t size = 0;
  BufferUsage bufferUsage = BufferUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

class Buffer {
  friend class Device;
  friend class CommandBuffer;
  friend class MemoryPool;
  friend struct CommandBufferImpl;
  friend struct BlasImpl; // TODO: remove me

public:
  Buffer() noexcept : m_handle(nullptr) {}
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

  uint64_t size() const noexcept;

  void commit() const;

  void *ptr() const;

  void set_name(const char *name) const noexcept;

private:
  Buffer(void *handle) noexcept : m_handle(handle) { assert(handle); }
  void *m_handle;
};
} // namespace strobe::gpu
