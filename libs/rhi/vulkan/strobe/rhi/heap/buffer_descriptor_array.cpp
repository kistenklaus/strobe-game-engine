#include "strobe/rhi/heap/buffer_descriptor_array.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_impl.hpp"

namespace strobe::rhi {

BufferDescriptorArray::BufferDescriptorArray(
    const BufferDescriptorArray &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BufferDescriptorArrayImpl>(m_handle);
  }
}

BufferDescriptorArray::BufferDescriptorArray(BufferDescriptorArray &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

BufferDescriptorArray &
BufferDescriptorArray::operator=(const BufferDescriptorArray &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BufferDescriptorArrayImpl>(o.m_handle);
  }
  unpin_void_handle<BufferDescriptorArrayImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

BufferDescriptorArray &
BufferDescriptorArray::operator=(BufferDescriptorArray &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BufferDescriptorArrayImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

BufferDescriptorArray::~BufferDescriptorArray() noexcept {
  unpin_void_handle<BufferDescriptorArrayImpl>(m_handle);
}

} // namespace strobe::rhi
