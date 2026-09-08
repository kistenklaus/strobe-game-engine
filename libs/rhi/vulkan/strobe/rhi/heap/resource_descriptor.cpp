#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_impl.hpp"
#include "strobe/rhi/objects/buffer_descriptor.hpp"

namespace strobe::rhi {

ResourceDescriptor::ResourceDescriptor(const ResourceDescriptor &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorImpl>(m_handle);
  }
}

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

ResourceDescriptor &
ResourceDescriptor::operator=(const ResourceDescriptor &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorImpl>(o.m_handle);
  }
  unpin_void_handle<ResourceDescriptorImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ResourceDescriptor &
ResourceDescriptor::operator=(ResourceDescriptor &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ResourceDescriptorImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ResourceDescriptor::~ResourceDescriptor() noexcept {
  unpin_void_handle<ResourceDescriptorImpl>(m_handle);
}

} // namespace strobe::rhi
