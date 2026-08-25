#pragma once

#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"

namespace strobe::rhi {

class MemoryPool {
  friend class Device;
  friend class Buffer;
  friend class CommandBuffer;
  friend struct MemoryAllocationImpl;

public:
  MemoryPool() noexcept : m_handle(nullptr) {}
  MemoryPool(const MemoryPool &) noexcept;
  MemoryPool(MemoryPool &&) noexcept;
  MemoryPool &operator=(const MemoryPool &) noexcept;
  MemoryPool &operator=(MemoryPool &&) noexcept;
  ~MemoryPool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  Buffer create_buffer(const BufferInfo &info,
                       const MemoryLifetime &lifetime = {}) const;

  Image create_image(const ImageCreateInfo &info,
                     const MemoryLifetime &lifetime = {});

  Blas create_blas(const BlasInfo &info, const MemoryLifetime &lifetime = {});

  void commit();

  bool memory_overlaps(const Buffer &buffer, const Buffer &rhs) const;

private:
  MemoryPool(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::rhi
