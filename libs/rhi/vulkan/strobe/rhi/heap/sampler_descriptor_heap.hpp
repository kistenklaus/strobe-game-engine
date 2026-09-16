#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/heap/descriptor_heap_bind_info.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
namespace strobe::rhi {

class SamplerDescriptorWizard;
class SamplerDescriptorArrayWizard;

class SamplerDescriptorHeap : Object<SamplerDescriptorHeap> {
  friend class Object<SamplerDescriptorHeap>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit SamplerDescriptorHeap(void *handle) noexcept;

  SamplerDescriptorWizard
  create_descriptor_wizard(const SamplerDescriptorInfo &info) noexcept;
  SamplerDescriptorArrayWizard create_descriptor_array_wizard(
      span<const SamplerDescriptorInfo> infos) noexcept;

  Buffer buffer() const noexcept;

  DescriptorHeapBindInfo bindInfo() const noexcept;

  vulkan::Context *ctx() const noexcept;
  Context context() const noexcept;
};

} // namespace strobe::rhi
