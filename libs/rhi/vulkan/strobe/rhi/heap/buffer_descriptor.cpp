#include "strobe/rhi/objects/buffer_descriptor.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"

namespace strobe::rhi {

BufferDescriptor::BufferDescriptor(const BufferDescriptor &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BufferDescriptorImpl>(m_handle);
  }
}

BufferDescriptor::BufferDescriptor(BufferDescriptor &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

BufferDescriptor &
BufferDescriptor::operator=(const BufferDescriptor &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BufferDescriptorImpl>(o.m_handle);
  }
  unpin_void_handle<BufferDescriptorImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

BufferDescriptor &BufferDescriptor::operator=(BufferDescriptor &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BufferDescriptorImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

BufferDescriptor::~BufferDescriptor() noexcept {
  unpin_void_handle<BufferDescriptorImpl>(m_handle);
}

} // namespace strobe::rhi
