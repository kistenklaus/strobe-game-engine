#pragma once

#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

VkPhysicalDevice select_physical_device(VkInstance instance,
                                        const ContextCreateInfo *info,
                                        const ContextProperties *props,
                                        const strobe::rhi::allocator_ref alloc);

} // namespace strobe::rhi::vulkan
