#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_array_impl.hpp"

namespace strobe::rhi {

void SamplerDescriptorArray::pin(void *handle) noexcept {
  pin_void_handle<SamplerDescriptorArrayImpl>(handle);
}
void SamplerDescriptorArray::unpin(void *handle) noexcept {
  unpin_void_handle<SamplerDescriptorArrayImpl>(handle);
}

} // namespace strobe::rhi
