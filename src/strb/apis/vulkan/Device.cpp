#include "strb/apis/vulkan/Device.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Device::Device(const PhysicalDevice &physicalDevice,
               const uint32_t graphicsQueueCount,
               const uint32_t transferQueueCount,
               const uint32_t computeQueueCount)
    : Device(physicalDevice, {}, graphicsQueueCount, transferQueueCount,
             computeQueueCount) {}
Device::Device(const PhysicalDevice &physicalDevice,
               strb::vector<strb::string> deviceExtentions,
               const uint32_t graphicsQueueCount,
               const uint32_t transferQueueCount,
               const uint32_t computeQueueCount)
    : physicalDevice(&physicalDevice) {
  strb::vector<const char *> deviceExtentions_cstr;
  for (const strb::string &str : deviceExtentions) {
    deviceExtentions_cstr.push_back(str.c_str());
  }
  // Fetch QueueFamilyIndex Information.
  this->queueFamilyIndicies = {};
  uint32_t n_queueFamilies;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n_queueFamilies,
                                           nullptr);
  strb::vector<VkQueueFamilyProperties> queueFamilies(n_queueFamilies);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n_queueFamilies,
                                           queueFamilies.data());
  for (uint64_t i = 0; i < queueFamilies.size(); i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      this->queueFamilyIndicies.graphicsIndex = i;
    } else if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      this->queueFamilyIndicies.computeIndex = i;
    } else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      this->queueFamilyIndicies.transferIndex = i;
    }
  }
  // Create Queue Creations structures
  strb::vector<VkDeviceQueueCreateInfo> queuesInfo;
  strb::vector<float> gfxPrios;
  if (graphicsQueueCount > 0) {
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    assert(this->queueFamilyIndicies.graphicsIndex.has_value());
    queueCreateInfo.queueFamilyIndex =
        this->queueFamilyIndicies.graphicsIndex.value();
    queueCreateInfo.queueCount = graphicsQueueCount;
    gfxPrios.resize(graphicsQueueCount, 1.0f);
    queueCreateInfo.pQueuePriorities = gfxPrios.data();
    queuesInfo.push_back(queueCreateInfo);
  }
  strb::vector<float> transferPrios;
  if (transferQueueCount > 0) {
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    assert(this->queueFamilyIndicies.transferIndex.has_value());
    queueCreateInfo.queueFamilyIndex =
        this->queueFamilyIndicies.transferIndex.value();
    queueCreateInfo.queueCount = transferQueueCount;
    transferPrios.resize(transferQueueCount, 1.0f);
    queueCreateInfo.pQueuePriorities = transferPrios.data();
    queuesInfo.push_back(queueCreateInfo);
  }
  strb::vector<float> computePrios;
  if (computeQueueCount > 0) {
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    assert(this->queueFamilyIndicies.computeIndex.has_value());
    queueCreateInfo.queueFamilyIndex =
        this->queueFamilyIndicies.computeIndex.value();
    queueCreateInfo.queueCount = computeQueueCount;
    computePrios.resize(computeQueueCount, 1.0f);
    queueCreateInfo.pQueuePriorities = computePrios.data();
    queuesInfo.push_back(queueCreateInfo);
  }
  // Create Device
  VkDeviceCreateInfo deviceCreateInfo;
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pNext = nullptr;
  deviceCreateInfo.flags = 0;
  deviceCreateInfo.queueCreateInfoCount = queuesInfo.size();
  deviceCreateInfo.pQueueCreateInfos = queuesInfo.data();
  deviceCreateInfo.enabledLayerCount = 0;
  deviceCreateInfo.ppEnabledLayerNames = nullptr;
  deviceCreateInfo.enabledExtensionCount = deviceExtentions_cstr.size();
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtentions_cstr.data();
  VkPhysicalDeviceFeatures features = {};
  deviceCreateInfo.pEnabledFeatures = &features;

  VkResult result =
      vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &this->device);
  ASSERT_VKRESULT(result);
  if (this->getGraphicsQueueFamilyIndex().has_value()) {
    for (uint32_t i = 0; i < graphicsQueueCount; i++) {
      VkQueue queue;
      vkGetDeviceQueue(this->device,
                       this->getGraphicsQueueFamilyIndex().value(), i, &queue);
      this->graphicsQueues.push_back(Queue(queue));
    }
  }
  if (this->getTransferQueueFamilyIndex().has_value()) {
    for (uint32_t i = 0; i < transferQueueCount; i++) {
      VkQueue queue;
      vkGetDeviceQueue(this->device,
                       this->getTransferQueueFamilyIndex().value(), i, &queue);
      this->transferQueues.push_back(Queue(queue));
    }
  }
  if (this->getComputeQueueFamilyIndex().has_value()) {
    for (uint32_t i = 0; i < computeQueueCount; i++) {
      VkQueue queue;
      vkGetDeviceQueue(this->device, this->getComputeQueueFamilyIndex().value(),
                       i, &queue);
      this->computeQueues.push_back(Queue(queue));
    }
  }
  assert(this->device != VK_NULL_HANDLE);
}

void Device::waitIdle() {
  assert(this->device != VK_NULL_HANDLE);
  VkResult result = vkDeviceWaitIdle(this->device);
  ASSERT_VKRESULT(result);
}

void Device::destroy() {
  assert(this->device != VK_NULL_HANDLE);
  vkDestroyDevice(this->device, nullptr);
  dexec(this->device = VK_NULL_HANDLE);
}

}  // namespace strb::vulkan
