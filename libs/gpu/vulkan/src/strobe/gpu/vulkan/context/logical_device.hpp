#pragma once

#include "strobe/gpu/vulkan/allocator.hpp"
#include "strobe/gpu/vulkan/context/context_properties.hpp"
#include "strobe/gpu/vulkan/context/driver_alloc.hpp"
#include "strobe/gpu/vulkan/context/select_queues.hpp"
#include "strobe/gpu/vulkan/device_info/device_info.hpp"
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

VkDevice
create_logical_device(VkPhysicalDevice physicalDevice,
                      const DeviceInfo<vulkan::allocator_ref> *deviceInfo,
                      span<const QueueLocation> queueLocations,
                      ContextProperties *properties,
                      const ContextCreateInfo *info, DriverAlloc *driverAlloc);

} // namespace strobe::gpu::vulkan
