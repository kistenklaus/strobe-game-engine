#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/debug_name.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

Buffer::Buffer(const Buffer &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BufferImpl>(m_handle);
  }
}

Buffer::Buffer(Buffer &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

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
  if (m_handle == nullptr) {
    return 0;
  }
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  return impl->size;
}

void *Buffer::ptr() const {
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  impl->commit();
  return impl->allocation.map();
}

void Buffer::set_name(const char *name) const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  vulkan::set_debug_name(impl->context.ctx(), impl->buffer, name);
}

} // namespace strobe::rhi
