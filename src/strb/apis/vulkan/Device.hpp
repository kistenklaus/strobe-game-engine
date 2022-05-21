#pragma once
#include "strb/apis/vulkan/PhysicalDevice.hpp"
#include "strb/apis/vulkan/Queue.hpp"
#include "strb/apis/vulkan/QueueFamilyIndicies.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

struct Queue;

struct Device {
 private:
  const PhysicalDevice* physicalDevice;
  VkDevice device;
  QueueFamilyIndicies queueFamilyIndicies;
  strb::vector<Queue> graphicsQueues;
  strb::vector<Queue> transferQueues;
  strb::vector<Queue> computeQueues;

 public:
  inline Device()
      : physicalDevice(nullptr),
        device(VK_NULL_HANDLE),
        queueFamilyIndicies({}) {}
  Device(const PhysicalDevice& physicalDevice,
         const uint32_t graphicsQueueCount, const uint32_t transferQueueCount,
         const uint32_t computeQueueCount);
  Device(const PhysicalDevice& physicalDevice,
         strb::vector<strb::string> deviceExtentions,
         const uint32_t graphicsQueueCount, const uint32_t transferQueueCount,
         const uint32_t computeQueueCount);
  void waitIdle();
  void destroy();
  inline strb::optional<uint32_t> getGraphicsQueueFamilyIndex() const {
    return queueFamilyIndicies.graphicsIndex;
  }
  inline strb::optional<uint32_t> getComputeQueueFamilyIndex() const {
    return queueFamilyIndicies.computeIndex;
  }
  inline strb::optional<uint32_t> getTransferQueueFamilyIndex() const {
    return queueFamilyIndicies.transferIndex;
  }
  inline Queue& getGraphicsQueue(const uint32_t index) {
    return this->graphicsQueues[index];
  }
  inline uint32_t getGraphicsQueueCount() {
    return this->graphicsQueues.size();
  }
  inline Queue& getTransferQueue(const uint32_t index) {
    return this->transferQueues[index];
  }
  inline uint32_t getTransferQueueCount() {
    return this->transferQueues.size();
  }
  inline Queue& getComputeQueue(const uint32_t index) {
    return this->computeQueues[index];
  }
  inline uint32_t getComputeQueueCount() { return this->computeQueues.size(); }
  inline operator VkDevice() const { return this->device; }
};

}  // namespace strb::vulkan
