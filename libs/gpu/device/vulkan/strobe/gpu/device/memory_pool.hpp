#pragma once

#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/memory_lifetime.hpp"
namespace strobe::gpu {

struct MemoryArena {};

class MemoryPool {
  friend class Device;
  friend class Buffer;
  friend struct MemoryAllocationImpl;

public:
  MemoryPool() noexcept : m_handle(nullptr) {}
  MemoryPool(const MemoryPool &) noexcept;
  MemoryPool(MemoryPool &&) noexcept;
  MemoryPool &operator=(const MemoryPool &) noexcept;
  MemoryPool &operator=(MemoryPool &&) noexcept;
  ~MemoryPool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  Buffer create_buffer(const BufferCreateInfo &info,
                       const MemoryLifetime &lifetime = {});

  void commit();

private:
  MemoryPool(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
