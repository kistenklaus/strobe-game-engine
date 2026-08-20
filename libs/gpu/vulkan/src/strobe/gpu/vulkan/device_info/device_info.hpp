#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/gpu/vulkan/device_info/device_extensions.hpp"
#include "strobe/gpu/vulkan/device_info/device_features.hpp"
#include "strobe/gpu/vulkan/device_info/device_properties.hpp"
#include "strobe/gpu/vulkan/device_info/memory_properties.hpp"
#include "strobe/gpu/vulkan/device_info/queue_family_properties.hpp"

namespace strobe::gpu::vulkan {

template <Allocator Alloc> struct DeviceInfo {
  using allocator = MonotonicResource<Alloc>;
  using allocator_ref = AllocatorReference<allocator>;

private:
  allocator m_allocator;

public:
  Vector<DeviceExtension<allocator_ref>, allocator_ref> supported_extensions;
  DeviceFeatures features;
  DeviceProperties<allocator_ref> properties;
  MemoryProperties<allocator_ref> memory_properties;
  Vector<QueueFamilyProperties, allocator_ref> queue_family_properties;

  static DeviceInfo query(VkInstance instance, VkPhysicalDevice physicalDevice,
                          uint32_t apiVersion, const Alloc &alloc = {}) {
    return DeviceInfo(instance, physicalDevice, apiVersion, alloc);
  }

private:
  explicit DeviceInfo(VkInstance instance, VkPhysicalDevice physicalDevice,
                      uint32_t apiVersion, const Alloc &alloc)
      : m_allocator{alloc}, //
        supported_extensions(query_device_extensions<allocator_ref>(
            physicalDevice, &m_allocator)),
        features(query_device_features<allocator_ref>(
            physicalDevice, apiVersion, supported_extensions)),
        properties(query_device_properties<allocator_ref>(
            instance, physicalDevice, &features, allocator_ref(&m_allocator))),
        memory_properties(query_memory_properties<allocator_ref>(physicalDevice,
                                                                 &m_allocator)),
        queue_family_properties(query_queue_family_properties<allocator_ref>(
            instance, physicalDevice, &m_allocator))

  {}
};

} // namespace strobe::gpu::vulkan
