#pragma once

#include "strobe/rhi/dma/async_transfer_cmd.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <limits>
namespace strobe::rhi {

class AsyncCopyEngine : Object<AsyncCopyEngine> {
public:
  explicit AsyncCopyEngine(void *handle) noexcept : Object(handle) {}
  AsyncCopyEngine() noexcept : Object(nullptr) {}
  AsyncCopyEngine(const AsyncCopyEngine &) noexcept;
  AsyncCopyEngine(AsyncCopyEngine &&) noexcept;
  AsyncCopyEngine &operator=(const AsyncCopyEngine &) noexcept;
  AsyncCopyEngine &operator=(AsyncCopyEngine &&) noexcept;
  ~AsyncCopyEngine() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const AsyncCopyEngine &lhs, const AsyncCopyEngine &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const AsyncCopyEngine &lhs, const AsyncCopyEngine &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  Timepoint
  async_copy(BufferOffset dst, BufferOffset src,
             uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
  Timepoint
  async_upload(BufferOffset dst, void *src,
               uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  AsyncTransferCmd async_cmd() noexcept;
};
} // namespace strobe::rhi
