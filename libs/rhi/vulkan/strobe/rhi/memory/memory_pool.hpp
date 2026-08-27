#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/memory/memory_requirements.hpp"
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

  void commit();

  bool memory_overlaps(const MemoryAllocation &lhs,
                       const MemoryAllocation &rhs) const noexcept;

  MemoryAllocation allocate_memory(const MemoryRequirements &requirements,
                                   const MemoryLifetime &lifetime) const;

  const Context &context() const noexcept;

  explicit MemoryPool(void *handle) noexcept : m_handle(handle) {}

private:
  void *m_handle;
};

} // namespace strobe::rhi
