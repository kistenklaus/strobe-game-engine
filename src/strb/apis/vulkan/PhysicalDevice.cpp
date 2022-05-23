#include "strb/apis/vulkan/PhysicalDevice.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

static void getPhysicalDeviceVRamSize(const VkPhysicalDevice physicalDevice,
                                      uint64_t *rVramSize) {
  uint64_t tmp = 0;
  VkPhysicalDeviceMemoryProperties physicalMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice,
                                      &physicalMemoryProperties);
  VkMemoryHeap *pHeaps = physicalMemoryProperties.memoryHeaps;
  strb::vector<VkMemoryHeap> heaps = strb::vector<VkMemoryHeap>(
      pHeaps, pHeaps + physicalMemoryProperties.memoryHeapCount);
  for (const VkMemoryHeap &heap : heaps) {
    if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
      tmp += heap.size;
    }
  }
  *rVramSize = tmp;
}

PhysicalDevice::PhysicalDevice(const Instance instance) {
  uint32_t n_physicalDevices;
  VkResult result =
      vkEnumeratePhysicalDevices(instance, &n_physicalDevices, nullptr);
  ASSERT_VKRESULT(result);
  if (n_physicalDevices == 0) {
    throw std::runtime_error("can't find any GPU");
  }
  strb::vector<VkPhysicalDevice> physicalDevices(n_physicalDevices);
  result = vkEnumeratePhysicalDevices(instance, &n_physicalDevices,
                                      physicalDevices.data());
  ASSERT_VKRESULT(result);

  uint64_t maxScore = 0;
  uint32_t bestGpuIndex = 0;
  for (uint64_t i = 0; i < n_physicalDevices; i++) {
    uint64_t score = 0;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    }
    uint64_t VRamSize;
    getPhysicalDeviceVRamSize(physicalDevices[i], &VRamSize);
    score += VRamSize;
    if (score > maxScore) {
      maxScore = score;
      bestGpuIndex = i;
    }
  }
  this->physicalDevice = physicalDevices[bestGpuIndex];
  vkGetPhysicalDeviceMemoryProperties(
      this->physicalDevice,
      &this->m_memoryProperties); // fetch memory properties
  assert(this->physicalDevice != VK_NULL_HANDLE);
}

uint64_t PhysicalDevice::getVRamSize() {
  assert(this->physicalDevice != VK_NULL_HANDLE);
  uint64_t VRamSize;
  getPhysicalDeviceVRamSize(this->physicalDevice, &VRamSize);
  return VRamSize;
}

uint32_t
PhysicalDevice::findMemoryTypeIndex(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties) const {
  for (uint32_t i = 0; i < this->m_memoryProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) &&
        (this->m_memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }
  throw strb::runtime_exception("failed to find suitable memory type");
}

} // namespace strb::vulkan
