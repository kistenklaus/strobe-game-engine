#include "strobe/rhi/objects/buffer_descriptor_array.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_impl.hpp"

namespace strobe::rhi {

ResourceDescriptorArray::ResourceDescriptorArray(
    const ResourceDescriptorArray &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorArrayImpl>(m_handle);
  }
}

ResourceDescriptorArray::ResourceDescriptorArray(
    ResourceDescriptorArray &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

ResourceDescriptorArray &
ResourceDescriptorArray::operator=(const ResourceDescriptorArray &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorArrayImpl>(o.m_handle);
  }
  unpin_void_handle<ResourceDescriptorArrayImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ResourceDescriptorArray &
ResourceDescriptorArray::operator=(ResourceDescriptorArray &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ResourceDescriptorArrayImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ResourceDescriptorArray::~ResourceDescriptorArray() noexcept {
  unpin_void_handle<ResourceDescriptorArrayImpl>(m_handle);
}

} // namespace strobe::rhi
