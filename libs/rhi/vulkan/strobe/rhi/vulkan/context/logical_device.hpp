#pragma once

#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/driver_alloc.hpp"
#include "strobe/rhi/vulkan/context/select_queues.hpp"
#include "strobe/rhi/vulkan/device_info/device_info.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

VkDevice create_logical_device(VkPhysicalDevice physicalDevice,
                               const DeviceInfo *deviceInfo,
                               span<const QueueLocation> queueLocations,
                               ContextProperties *properties,
                               const ContextCreateInfo *info,
                               DriverAlloc *driverAlloc);

} // namespace strobe::rhi::vulkan
