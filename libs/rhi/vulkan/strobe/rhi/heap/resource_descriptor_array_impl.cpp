#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"

namespace strobe::rhi {

ResourceDescriptorArrayImpl::~ResourceDescriptorArrayImpl() noexcept {
  assert(index != std::numeric_limits<uint32_t>::max());
  auto *heap_impl = object_handle_ptr<ResourceDescriptorHeapImpl>(heap);
  heap_impl->layout.free_range(index, size);

  assert(m_descriptors != nullptr);
  std::destroy_n(m_descriptors, size);

  heap_impl->alloc.deallocate(m_descriptors, size * sizeof(ResourceDescriptor),
                              alignof(ResourceDescriptor));
}

} // namespace strobe::rhi
