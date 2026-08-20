#pragma once

#include "strobe/gpu/device/buffer_usage.hpp"
#include "strobe/gpu/device/memory_usage.hpp"
#include <cassert>
#include <cstdint>
namespace strobe::gpu {

struct BufferCreateInfo {
  uint64_t size = 0;
  BufferUsage usage = BufferUsage::none;
  MemoryUsage memory_usage = MemoryUsage::automatic;
};

class Buffer {
  friend class Device;
  friend class CommandBuffer;
  friend class MemoryPool;
  friend struct CommandBufferImpl;

public:
  Buffer() noexcept : m_handle(nullptr) {}
  Buffer(const Buffer &) noexcept;
  Buffer(Buffer &&) noexcept;
  Buffer &operator=(const Buffer &) noexcept;
  Buffer &operator=(Buffer &&) noexcept;
  ~Buffer() noexcept;

  uint64_t size() const noexcept;

  void commit();

  void *ptr();

private:
  Buffer(void *handle) noexcept : m_handle(handle) {
    assert(handle);
  }
  void *m_handle;
};
} // namespace strobe::gpu
