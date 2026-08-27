#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/bvh/scratch_buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include <utility>

namespace strobe::rhi {

ScratchBuffer::ScratchBuffer(const ScratchBuffer &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ScratchBufferImpl>(m_handle);
  }
}

ScratchBuffer::ScratchBuffer(ScratchBuffer &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

ScratchBuffer &ScratchBuffer::operator=(const ScratchBuffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ScratchBufferImpl>(o.m_handle);
  }
  unpin_void_handle<ScratchBufferImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ScratchBuffer &ScratchBuffer::operator=(ScratchBuffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ScratchBufferImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ScratchBuffer::~ScratchBuffer() noexcept {
  unpin_void_handle<ScratchBufferImpl>(m_handle);
}

Buffer ScratchBuffer::scratch() const noexcept {
  return void_handle_ptr<ScratchBufferImpl>(m_handle)->scratch();
}


void ScratchBuffer::require(uint64_t size) const noexcept {
  void_handle_ptr<ScratchBufferImpl>(m_handle)->require(size);
}

} // namespace strobe::rhi
