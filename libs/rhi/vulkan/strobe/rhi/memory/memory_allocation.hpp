#pragma once

namespace strobe::rhi {

struct MemoryAllocation {
  friend class MemoryPool;
  friend struct MemoryPoolImpl;
  friend class Buffer;
  friend struct BufferImpl;
  friend class CommandBuffer;

public:
  MemoryAllocation() noexcept = default;

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

private:
  explicit MemoryAllocation(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::rhi
