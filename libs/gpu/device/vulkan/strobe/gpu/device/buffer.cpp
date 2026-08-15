#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

namespace strobe::gpu {

Buffer::Buffer(const Buffer &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BufferImpl>(m_handle);
  }
}

Buffer::Buffer(Buffer &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Buffer &Buffer::operator=(const Buffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BufferImpl>(o.m_handle);
  }
  unpin_void_handle<BufferImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Buffer &Buffer::operator=(Buffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BufferImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Buffer::~Buffer() noexcept { unpin_void_handle<BufferImpl>(m_handle); }

uint64_t Buffer::size() const noexcept {
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  return impl->size;
}

} // namespace strobe::gpu
