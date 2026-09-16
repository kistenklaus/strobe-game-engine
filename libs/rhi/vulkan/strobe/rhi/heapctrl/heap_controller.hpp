#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/resource_descriptor_array.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"

namespace strobe::rhi {

class HeapController : public Object<HeapController> {
  friend class Object<HeapController>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit HeapController(void *handle) noexcept : Object(handle) {}

  ResourceDescriptor
  create_resource_descriptor(const ResourceDescriptorInfo &info) noexcept;

  ResourceDescriptorArray create_resource_descriptor_array(
      span<const ResourceDescriptorInfo> infos) noexcept;

  SamplerDescriptor
  create_sampler_descriptor(const SamplerDescriptorInfo &info) noexcept;
  SamplerDescriptorArray create_sampler_descriptor_array(
      span<const SamplerDescriptorInfo> infos) noexcept;
};

} // namespace strobe::rhi
