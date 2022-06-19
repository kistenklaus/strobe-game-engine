#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct CommandBuffer {
  friend struct CommandPool;

 private:
  VkCommandBuffer commandBuffer;
  inline CommandBuffer(const VkCommandBuffer commandBuffer)
      : commandBuffer(commandBuffer) {}

 public:
  // pls dont use this explicitly
  inline CommandBuffer() : commandBuffer(VK_NULL_HANDLE) {}
  inline operator VkCommandBuffer() const { return this->commandBuffer; }
  void begin();
  void end();

 public:
};

}  // namespace strb::vulkan
