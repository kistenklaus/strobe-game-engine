#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/heap/descriptor_heap_bind_info.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"

namespace strobe::rhi {

// fwd declarations.
class ResourceDescriptorWizard;
class ResourceDescriptorArrayWizard;

class ResourceDescriptorHeap : Object<ResourceDescriptorHeap> {
public:
  explicit ResourceDescriptorHeap(void *handle) noexcept : Object(handle) {}
  ResourceDescriptorHeap() noexcept : Object(nullptr) {}
  ResourceDescriptorHeap(const ResourceDescriptorHeap &) noexcept;
  ResourceDescriptorHeap(ResourceDescriptorHeap &&) noexcept;
  ResourceDescriptorHeap &operator=(const ResourceDescriptorHeap &) noexcept;
  ResourceDescriptorHeap &operator=(ResourceDescriptorHeap &&) noexcept;
  ~ResourceDescriptorHeap() noexcept;

  ResourceDescriptorWizard
  create_descriptor_wizard(const ResourceDescriptorInfo &info) noexcept;

  ResourceDescriptorArrayWizard create_descriptor_array_wizard(
      span<const ResourceDescriptorInfo> infos) noexcept;

  Buffer buffer() const noexcept;

  DescriptorHeapBindInfo bindInfo() const noexcept;

  vulkan::Context *ctx() const noexcept;
  Context context() const noexcept;
};

} // namespace strobe::rhi
