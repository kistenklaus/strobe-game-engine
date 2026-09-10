#include "strobe/rhi/objects/resource_descriptor_array.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_impl.hpp"

namespace strobe::rhi {

void ResourceDescriptorArray::pin(void *handle) noexcept {
  pin_void_handle<ResourceDescriptorArrayImpl>(handle);
}
void ResourceDescriptorArray::unpin(void *handle) noexcept {
  unpin_void_handle<ResourceDescriptorArrayImpl>(handle);
}

} // namespace strobe::rhi
