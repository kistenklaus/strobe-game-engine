#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_impl.hpp"

namespace strobe::rhi {

void SamplerDescriptor::pin(void *handle) noexcept {
  pin_void_handle<SamplerDescriptorImpl>(handle);
}

void SamplerDescriptor::unpin(void *handle) noexcept {
  unpin_void_handle<SamplerDescriptorImpl>(handle);
}

} // namespace strobe::rhi
