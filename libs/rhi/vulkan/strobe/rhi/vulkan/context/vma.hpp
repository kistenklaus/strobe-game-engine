#pragma once

#include "strobe/rhi/vulkan/context/driver_alloc.hpp"
#include <vk_mem_alloc.h>

namespace strobe::rhi::vulkan {

VmaAllocator create_vma(VkInstance instance, VkPhysicalDevice physicalDevice,
                        VkDevice device, uint32_t apiVersion,
                        const DriverAlloc *driverAlloc);

} // namespace strobe::rhi::vulkan
