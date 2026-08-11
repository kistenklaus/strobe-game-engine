#include "strobe/gpu/vulkan/descriptor_set_layout.hpp"
#include <stdexcept>

namespace strobe::gpu::vulkan {

DescriptorSetLayout
create_descriptor_set_layout(Context *context,
                             const DescriptorSetLayoutInfo &info) {
  assert(context != nullptr);

  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkDescriptorSetLayoutBinding) * 24 +
      sizeof(VkDescriptorBindingFlags) * 24;

  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;

  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  const uint32_t count = static_cast<uint32_t>(info.bindings.size());

  Vector<VkDescriptorSetLayoutBinding, scratch_allocator_ref> bindings{
      count, &scratch};

  Vector<VkDescriptorBindingFlags, scratch_allocator_ref> bindingFlags{
      count, &scratch};

  bool hasBindingFlags = false;

  for (uint32_t i = 0; i < count; ++i) {
    const DescriptorSetLayoutBinding &binding = info.bindings[i];

    if (binding.count == 0) {
      throw std::runtime_error("Failed to create descriptor set layout: "
                               "descriptor count must be nonzero");
    }

    VkSampler *immutableSamplers = nullptr;

    if (!binding.immutableSamplers.empty()) {
      if (binding.type != VK_DESCRIPTOR_TYPE_SAMPLER &&
          binding.type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        throw std::runtime_error(
            "Failed to create descriptor set layout: "
            "immutable samplers require a sampler binding");
      }

      if (binding.immutableSamplers.size() != binding.count) {
        throw std::runtime_error("Failed to create descriptor set layout: "
                                 "invalid amount of immutable samplers");
      }

      immutableSamplers = static_cast<VkSampler *>(scratch.allocate(
          binding.count * sizeof(VkSampler), alignof(VkSampler)));

      for (uint32_t j = 0; j < binding.count; ++j) {
        assert(binding.immutableSamplers[j]);

        immutableSamplers[j] = binding.immutableSamplers[j].handle;
      }
    }

    bindings[i] = VkDescriptorSetLayoutBinding{
        .binding = binding.binding,
        .descriptorType = binding.type,
        .descriptorCount = binding.count,
        .stageFlags = binding.stage,
        .pImmutableSamplers = immutableSamplers,
    };

    bindingFlags[i] = binding.flags;
    hasBindingFlags |= binding.flags != 0;
  }

  const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .pNext = nullptr,
      .bindingCount = count,
      .pBindingFlags = bindingFlags.empty() ? nullptr : bindingFlags.data(),
  };

  const VkDescriptorSetLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = hasBindingFlags ? &bindingFlagsInfo : nullptr,
      .flags = info.flags,
      .bindingCount = count,
      .pBindings = bindings.empty() ? nullptr : bindings.data(),
  };

  DescriptorSetLayout layout{};

  const VkResult result = vkCreateDescriptorSetLayout(
      context->device(), &createInfo, context->driver_alloc(), &layout.handle);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout");
  }

  return layout;
}

void destroy_descriptor_set_layout(Context *context,
                                   DescriptorSetLayout layout) noexcept {
  assert(context != nullptr);
  assert(layout);
  vkDestroyDescriptorSetLayout(context->device(), layout.handle,
                               context->driver_alloc());
}

} // namespace strobe::gpu::vulkan
