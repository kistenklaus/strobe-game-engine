#include "strobe/rhi/heap/sampler_descriptor_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include <limits>

namespace strobe::rhi {

SamplerDescriptorImpl::~SamplerDescriptorImpl() noexcept {
  assert(index != std::numeric_limits<uint32_t>::max());
  object_handle_ptr<SamplerDescriptorHeapImpl>(heap)->layout.free_range(index,
                                                                        1);
}

} // namespace strobe::rhi
