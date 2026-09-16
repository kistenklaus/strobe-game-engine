#include "strobe/rhi/heap/sampler_descriptor_array_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include <limits>
#include <memory>

namespace strobe::rhi {

SamplerDescriptorArrayImpl::~SamplerDescriptorArrayImpl() noexcept {
  assert(index != std::numeric_limits<uint32_t>::max());
  auto *heap_impl = object_handle_ptr<SamplerDescriptorHeapImpl>(heap);
  heap_impl->layout.free_range(index, size);

  assert(m_descriptors != nullptr);
  std::destroy_n(m_descriptors, size);
  heap_impl->alloc.deallocate(m_descriptors, size * sizeof(SamplerDescriptor),
                              alignof(SamplerDescriptor));
}

} // namespace strobe::rhi
