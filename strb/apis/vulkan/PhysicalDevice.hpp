#pragma once
#include "strb/apis/vulkan/Instance.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct PhysicalDevice {
 private:
  VkPhysicalDevice physicalDevice;

 public:
  inline PhysicalDevice() : physicalDevice(VK_NULL_HANDLE) {}
  PhysicalDevice(const Instance instance);
  uint64_t getVRamSize();
  inline operator VkPhysicalDevice() const { return this->physicalDevice; }
};

}  // namespace strb::vulkan
