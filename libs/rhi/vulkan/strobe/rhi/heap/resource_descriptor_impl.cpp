#include "strobe/rhi/heap/resource_descriptor_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"

namespace strobe::rhi {

ResourceDescriptorImpl::~ResourceDescriptorImpl() noexcept {
  assert(index != std::numeric_limits<uint32_t>::max());
  object_handle_ptr<ResourceDescriptorHeapImpl>(heap)->layout.free_range(index,
                                                                         1);
}
} // namespace strobe::rhi
