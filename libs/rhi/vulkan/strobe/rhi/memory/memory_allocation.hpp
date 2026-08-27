#pragma once

#include "strobe/rhi/memory/memory_binding.hpp"
#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

struct MemoryAllocation : Object<MemoryAllocation> {
  friend class MemoryPool;
  friend struct MemoryPoolImpl;
  friend class Buffer;
  friend struct BufferImpl;
  friend class CommandBuffer;

public:
  explicit MemoryAllocation(void *handle) noexcept : Object(handle) {}
  MemoryAllocation() noexcept : Object(nullptr) {}

  MemoryAllocation(const MemoryAllocation &) noexcept;
  MemoryAllocation(MemoryAllocation &&) noexcept;

  MemoryAllocation &operator=(const MemoryAllocation &) noexcept;
  MemoryAllocation &operator=(MemoryAllocation &&) noexcept;

  ~MemoryAllocation() noexcept;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return m_handle != nullptr;
  }

  void *map() const ;
  void flush() const;
  void invalidate() const;
  bool commit() const;

  const MemoryBinding& binding() const;
};

} // namespace strobe::rhi
