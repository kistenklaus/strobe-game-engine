#pragma once
#include "strb/apis/vulkan/Instance.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct PhysicalDevice {
private:
  VkPhysicalDevice physicalDevice;
  VkPhysicalDeviceMemoryProperties m_memoryProperties;

public:
  inline PhysicalDevice() : physicalDevice(VK_NULL_HANDLE) {}
  PhysicalDevice(const Instance instance);
  uint64_t getVRamSize();
  inline operator VkPhysicalDevice() const { return this->physicalDevice; }
  uint32_t findMemoryTypeIndex(uint32_t typeFilter, uint32_t properties) const;
};

} // namespace strb::vulkan
