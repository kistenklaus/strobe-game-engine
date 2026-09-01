#include "strobe/rhi/dma/DMA.hpp"
#include "strobe/rhi/dma/DMAImpl.hpp"
#include "strobe/rhi/handle.hpp"
#include <limits>

namespace strobe::rhi {

DMA::DMA(const DMA &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<DMAImpl>(m_handle);
  }
}

DMA::DMA(DMA &&o) noexcept : Object(std::exchange(o.m_handle, nullptr)) {}

DMA &DMA::operator=(const DMA &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<DMAImpl>(o.m_handle);
  }
  unpin_void_handle<DMAImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

DMA &DMA::operator=(DMA &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<DMAImpl>(o.m_handle);
  }
  unpin_void_handle<DMAImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

DMA::~DMA() noexcept { unpin_void_handle<DMAImpl>(m_handle); }

Timepoint DMA::async_copy(BufferOffset dst, BufferOffset src,
                          uint64_t size) noexcept {
  auto *impl = void_handle_ptr<DMAImpl>(m_handle);
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = std::min(dst.buffer.size() - dst.offset,
                    src.buffer.size() - src.offset);
  }
  return impl->async_copy(dst, src, size);
}

Timepoint DMA::async_upload(BufferOffset dst, void *src,
                            uint64_t size) noexcept {

  auto *impl = void_handle_ptr<DMAImpl>(m_handle);
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = dst.buffer.size() - dst.offset;
  }
  return impl->async_upload(dst, src, size);
}

} // namespace strobe::rhi
