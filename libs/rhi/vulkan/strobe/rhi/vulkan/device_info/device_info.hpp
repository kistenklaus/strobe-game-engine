#pragma once

#include "strobe/rhi/vulkan/device_info/device_extensions.hpp"
#include "strobe/rhi/vulkan/device_info/device_features.hpp"
#include "strobe/rhi/vulkan/device_info/device_properties.hpp"
#include "strobe/rhi/vulkan/device_info/memory_properties.hpp"
#include "strobe/rhi/vulkan/device_info/queue_family_properties.hpp"

namespace strobe::rhi::vulkan {

struct DeviceInfo {
public:
  Vector<DeviceExtension, strobe::rhi::allocator_ref> supported_extensions;
  DeviceFeatures features;
  DeviceProperties properties;
  MemoryProperties memory_properties;
  Vector<QueueFamilyProperties, strobe::rhi::allocator_ref>
      queue_family_properties;

  static DeviceInfo query(VkInstance instance, VkPhysicalDevice physicalDevice,
                          uint32_t apiVersion,
                          const strobe::rhi::allocator_ref &alloc) {
    ZoneScopedN("context/query-physical-device-info");
    return DeviceInfo(instance, physicalDevice, apiVersion, alloc);
  }

private:
  explicit DeviceInfo(VkInstance instance, VkPhysicalDevice physicalDevice,
                      uint32_t apiVersion,
                      const strobe::rhi::allocator_ref &alloc)
      : supported_extensions(
            details::query_device_extensions(physicalDevice, alloc)),
        features(details::query_device_features(physicalDevice, apiVersion,
                                                supported_extensions)),
        properties(details::query_device_properties(instance, physicalDevice,
                                                    &features, alloc)),
        memory_properties(
            details::query_memory_properties(physicalDevice, alloc)),
        queue_family_properties(details::query_queue_family_properties(
            instance, physicalDevice, alloc))

  {}
};

} // namespace strobe::rhi::vulkan
