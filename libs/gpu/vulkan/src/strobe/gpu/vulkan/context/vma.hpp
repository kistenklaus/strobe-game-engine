#pragma once

#include "strobe/gpu/vulkan/context/driver_alloc.hpp"
#include <vk_mem_alloc.h>

namespace strobe::gpu::vulkan {

VmaAllocator create_vma(VkInstance instance, VkPhysicalDevice physicalDevice,
                        VkDevice device, uint32_t apiVersion,
                        const DriverAlloc *driverAlloc);



} // namespace strobe::gpu::vulkan
