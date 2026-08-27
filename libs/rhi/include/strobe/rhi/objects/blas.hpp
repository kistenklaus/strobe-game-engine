#pragma once

#include "strobe/rhi/objects/buffer.hpp"

namespace strobe::rhi {

struct Blas : Object<Blas> {
  friend class Device;
  friend class MemoryPool;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  Blas() noexcept : Object(nullptr) {}
  Blas(const Blas &) noexcept;
  Blas(Blas &&) noexcept;
  Blas &operator=(const Blas &) noexcept;
  Blas &operator=(Blas &&) noexcept;
  ~Blas() noexcept;
  explicit operator bool() const noexcept { return m_handle; }
  friend bool operator==(const Blas &lhs, const Blas &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Blas &lhs, const Blas &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  explicit Blas(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi
