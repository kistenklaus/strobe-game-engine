#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/sampler.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct DescriptorSetLayout {
  VkDescriptorSetLayout handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct DescriptorSetLayoutBinding {
  uint32_t binding = 0;
  VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  uint32_t count = 1;
  VkShaderStageFlags stage = VK_SHADER_STAGE_ALL;
  span<const Sampler> immutableSamplers = {};
  VkDescriptorBindingFlags flags = 0;
};

struct DescriptorSetLayoutInfo {
  VkDescriptorSetLayoutCreateFlags flags = 0;
  span<const DescriptorSetLayoutBinding> bindings = {};
};

DescriptorSetLayout
create_descriptor_set_layout(Context *context,
                             const DescriptorSetLayoutInfo &info = {});

void destroy_descriptor_set_layout(Context *context,
                                   DescriptorSetLayout layout) noexcept;

} // namespace strobe::gpu::vulkan
