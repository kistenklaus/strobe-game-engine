#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace strobe::rhi::vulkan {

enum feature : uint8_t {
  disable,
  optional,
  required,

};

struct QueueDescription {
  VkQueueFlags require = 0;
  VkQueueFlags prefer = 0;
  VkQueueFlags exclude = 0;
  feature present = optional;
  feature available = required;
  float priority = 1.0f;
};

struct ContextCreateInfo {
  feature debug_utils = disable;
  feature surface = required;

  feature swapchain = required;
  feature timeline_semaphore = required;

  feature shaderObjects = required;
  feature hostQueryReset = required;
  feature calibratedTimestamps = required;
  feature bufferDeviceAddress = required;
  // sync2 is always required

  feature raytracingPipeline = required;
  feature accelerationStructure = required;
  feature deferredHostOperations = required;
  feature rayQuery = required;

  feature descriptorHeap = required;
  feature shader_untyped_pointers = required;

  uint32_t queue_count = 0;
  const QueueDescription *pQueues;
};

} // namespace strobe::rhi::vulkan
