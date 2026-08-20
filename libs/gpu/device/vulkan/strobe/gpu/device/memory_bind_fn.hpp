#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
namespace strobe::gpu {


struct MemoryBindFn {
  void *obj;
  void (*bind)(void *obj, VmaAllocation allocation, VkDeviceSize offset);
};

} // namespace strobe::gpu
