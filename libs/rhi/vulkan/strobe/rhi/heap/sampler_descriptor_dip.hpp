#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"
namespace strobe::rhi {

struct SamplerDescriptorDIP {
  void *pUserData;
  SamplerDescriptor (*fnp_create_descriptor)(
      void *pUserData, const SamplerDescriptorInfo &info) noexcept;
  SamplerDescriptorArray (*fnp_create_descriptor_array)(
      void *pUserData, span<const SamplerDescriptorInfo> infos) noexcept;

  SamplerDescriptor
  create_descriptor(const SamplerDescriptorInfo &info) const noexcept {
    return fnp_create_descriptor(pUserData, info);
  }

  SamplerDescriptorArray create_descriptor_array(
      span<const SamplerDescriptorInfo> infos) const noexcept {
    return fnp_create_descriptor_array(pUserData, infos);
  }
};

} // namespace strobe::rhi
