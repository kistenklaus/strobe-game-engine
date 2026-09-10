#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_impl.hpp"

namespace strobe::rhi {

void ResourceDescriptor::pin(void *handle) noexcept {
  pin_void_handle<ResourceDescriptorImpl>(handle);
}

void ResourceDescriptor::unpin(void *handle) noexcept {
  unpin_void_handle<ResourceDescriptorImpl>(handle);
}

} // namespace strobe::rhi
