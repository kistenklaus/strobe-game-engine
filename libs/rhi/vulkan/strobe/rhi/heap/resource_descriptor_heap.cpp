#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_wizard.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"

namespace strobe::rhi {

void ResourceDescriptorHeap::pin(void *handle) noexcept {
  pin_void_handle<ResourceDescriptorHeapImpl>(handle);
}

void ResourceDescriptorHeap::unpin(void *handle) noexcept {
  unpin_void_handle<ResourceDescriptorHeapImpl>(handle);
}

ResourceDescriptorWizard ResourceDescriptorHeap::create_descriptor_wizard(
    const ResourceDescriptorInfo &info) noexcept {
  auto *impl = void_handle_ptr<ResourceDescriptorHeapImpl>(m_handle);
  uint32_t index = impl->layout.alloc_range(1);
  return ResourceDescriptorWizard{*this, index, info};
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
