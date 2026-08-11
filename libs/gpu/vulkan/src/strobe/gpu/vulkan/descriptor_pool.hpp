#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct DescriptorPool {
  VkDescriptorPool handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct DescriptorPoolInfo {
  VkDescriptorPoolCreateFlags flags = 0;
  uint32_t maxSets = 0;
  span<const VkDescriptorPoolSize> poolSizes = {};
};

DescriptorPool create_descriptor_pool(Context *context,
                                      const DescriptorPoolInfo &info = {});

void destroy_descriptor_pool(Context *context, DescriptorPool pool) noexcept;

void reset_descriptor_pool(Context *context, DescriptorPool pool);

} // namespace strobe::gpu::vulkan
