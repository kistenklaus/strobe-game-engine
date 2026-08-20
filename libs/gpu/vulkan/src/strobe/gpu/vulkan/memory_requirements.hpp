#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace strobe::gpu::vulkan {

struct MemoryRequirements {
  VkDeviceSize size;
  VkDeviceSize alignment;
  uint32_t memoryTypeBits;
  bool prefersDedicated;
  bool requiresDedicated;
};

}
