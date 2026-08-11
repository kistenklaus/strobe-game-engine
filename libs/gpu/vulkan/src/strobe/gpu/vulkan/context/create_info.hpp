#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>
namespace strobe::gpu::vulkan {

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
  feature surface = disable;

  feature swapchain = disable;
  feature timeline_semaphore = disable;

  uint32_t queue_count = 0;
  const QueueDescription *pQueues;
};

} // namespace strobe::gpu::vulkan
