#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <limits>
namespace strobe::rhi {

class DMA : Object<DMA> {
public:
  explicit DMA(void *handle) noexcept : Object(handle) {}
  DMA() noexcept : Object(nullptr) {}
  DMA(const DMA &) noexcept;
  DMA(DMA &&) noexcept;
  DMA &operator=(const DMA &) noexcept;
  DMA &operator=(DMA &&) noexcept;
  ~DMA() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const DMA &lhs, const DMA &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const DMA &lhs, const DMA &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  Timepoint
  async_copy(BufferOffset dst, BufferOffset src,
             uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
  Timepoint
  async_upload(BufferOffset dst, void *src,
               uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
};
} // namespace strobe::rhi
