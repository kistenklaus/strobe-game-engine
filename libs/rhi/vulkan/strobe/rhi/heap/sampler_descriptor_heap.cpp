#include "strobe/rhi/heap/sampler_descriptor_heap.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_array_wizard.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/sampler_descriptor_wizard.hpp"

namespace strobe::rhi {

void SamplerDescriptorHeap::pin(void *handle) noexcept {
  pin_void_handle<SamplerDescriptorHeapImpl>(handle);
}
void SamplerDescriptorHeap::unpin(void *handle) noexcept {
  unpin_void_handle<SamplerDescriptorHeapImpl>(handle);
}

SamplerDescriptorHeap::SamplerDescriptorHeap(void *handle) noexcept
    : Object(handle) {}

SamplerDescriptorWizard SamplerDescriptorHeap::create_descriptor_wizard(
    const SamplerDescriptorInfo &info) noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  uint32_t index = impl->layout.alloc_range(1);
  return SamplerDescriptorWizard{*this, index, info};
}

SamplerDescriptorArrayWizard
SamplerDescriptorHeap::create_descriptor_array_wizard(
    span<const SamplerDescriptorInfo> infos) noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  uint32_t index = impl->layout.alloc_range(infos.size());
  return SamplerDescriptorArrayWizard{*this, index, infos, impl->alloc};
}

Buffer SamplerDescriptorHeap::buffer() const noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  return impl->buffer();
}

DescriptorHeapBindInfo SamplerDescriptorHeap::bindInfo() const noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  return impl->bindInfo();
}

vulkan::Context *SamplerDescriptorHeap::ctx() const noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  return impl->context.ctx();
}

Context SamplerDescriptorHeap::context() const noexcept {
  auto *impl = void_handle_ptr<SamplerDescriptorHeapImpl>(m_handle);
  return impl->context;
}

} // namespace strobe::rhi
