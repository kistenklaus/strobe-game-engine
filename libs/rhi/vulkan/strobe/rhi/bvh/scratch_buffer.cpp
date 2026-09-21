#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/bvh/scratch_buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

void ScratchBuffer::pin(void *handle) noexcept {
  pin_void_handle<ScratchBufferImpl>(handle);
}

void ScratchBuffer::unpin(void *handle) noexcept {
  unpin_void_handle<ScratchBufferImpl>(handle);
}

Buffer ScratchBuffer::buffer() const noexcept {
  return void_handle_ptr<ScratchBufferImpl>(m_handle)->scratch();
}

void ScratchBuffer::require(uint64_t size) const noexcept {
  void_handle_ptr<ScratchBufferImpl>(m_handle)->require(size);
}

} // namespace strobe::rhi
