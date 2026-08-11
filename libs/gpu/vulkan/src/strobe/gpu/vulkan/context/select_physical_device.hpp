#pragma once

#include "strobe/gpu/vulkan/context/context_properties.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

VkPhysicalDevice select_physical_device(VkInstance instance,
                                        const ContextCreateInfo *info,
                                        const ContextProperties *props);

} // namespace strobe::gpu::vulkan
