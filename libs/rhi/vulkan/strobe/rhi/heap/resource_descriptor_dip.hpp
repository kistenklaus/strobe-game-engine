#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/resource_descriptor_array.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"
namespace strobe::rhi {

struct ResourceDescriptorDIP {
  void *pUserData;
  ResourceDescriptor (*fnp_create_descriptor)(
      void *pUserData, const ResourceDescriptorInfo &info) noexcept;
  ResourceDescriptorArray (*fnp_create_descriptor_array)(
      void *pUserData, span<const ResourceDescriptorInfo> infos) noexcept;

  ResourceDescriptor
  create_descriptor(const ResourceDescriptorInfo &info) const noexcept {
    return fnp_create_descriptor(pUserData, info);
  }

  ResourceDescriptorArray create_descriptor_array(
      span<const ResourceDescriptorInfo> infos) const noexcept {
    return fnp_create_descriptor_array(pUserData, infos);
  }
};

} // namespace strobe::rhi
