#pragma once
#include <cinttypes>

#include "strb/apis/vulkan/CommandBuffer.hpp"
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/QueueFamily.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct CommandPool {
 private:
  const Device* device;
  QueueFamily queueFamily;
  VkCommandPool commandPool;
  strb::vector<CommandBuffer> allocatedBuffers;

 public:
  inline CommandPool() : device(nullptr), commandPool(VK_NULL_HANDLE) {}
  CommandPool(const Device& device, const QueueFamily queueFamily);
  void destroy();
  const strb::vector<CommandBuffer> allocate(const uint32_t commandBufferCount);
  inline operator VkCommandPool() { return this->commandPool; }
  void free(const strb::vector<CommandBuffer>& commandBuffers);
  void recreate();
};

}  // namespace strb::vulkan
