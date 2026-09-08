#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_wizard.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"

namespace strobe::rhi {

ResourceDescriptorHeap::ResourceDescriptorHeap(
    const ResourceDescriptorHeap &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorHeapImpl>(m_handle);
  }
}

ResourceDescriptorHeap::ResourceDescriptorHeap(
    ResourceDescriptorHeap &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

ResourceDescriptorHeap &
ResourceDescriptorHeap::operator=(const ResourceDescriptorHeap &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ResourceDescriptorHeapImpl>(o.m_handle);
  }
  unpin_void_handle<ResourceDescriptorHeapImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ResourceDescriptorHeap &
ResourceDescriptorHeap::operator=(ResourceDescriptorHeap &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ResourceDescriptorHeapImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ResourceDescriptorHeap::~ResourceDescriptorHeap() noexcept {
  unpin_void_handle<ResourceDescriptorHeapImpl>(m_handle);
}

ResourceDescriptorWizard ResourceDescriptorHeap::create_descriptor_wizard(
    const ResourceDescriptorInfo &buffer) noexcept {
  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  uint32_t index = impl->layout.alloc_range(1);
  return ResourceDescriptorWizard{*this, index, buffer};
}

ResourceDescriptorArrayWizard
ResourceDescriptorHeap::create_descriptor_array_wizard(
    span<const ResourceDescriptorInfo> infos) noexcept {

  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  uint32_t index = impl->layout.alloc_range(infos.size());
  return ResourceDescriptorArrayWizard{*this, index, infos, impl->alloc};
}

Buffer ResourceDescriptorHeap::buffer() const noexcept {
  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  return impl->buffer();
}

DescriptorHeapBindInfo ResourceDescriptorHeap::bindInfo() const noexcept {
  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  return impl->bindInfo();
}

vulkan::Context *ResourceDescriptorHeap::ctx() const noexcept {
  return context().ctx();
}

Context ResourceDescriptorHeap::context() const noexcept {
  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  return impl->context;
}

} // namespace strobe::rhi
