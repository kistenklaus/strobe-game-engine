#include "strobe/gpu/vulkan/command_buffer.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

CommandBuffer alloc_command_buffer(Context *context,
                                   const CommandBufferInfo &info) {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = info.pool.handle,
      .level = info.level,
      .commandBufferCount = 1,
  };

  CommandBuffer cmd;
  VkResult result =
      vkAllocateCommandBuffers(context->device(), &allocInfo, &cmd.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffer");
  }
  return cmd;
}

void free_command_buffer(Context *context, CommandPool pool,
                         CommandBuffer cmd) noexcept {
  assert(context != nullptr);
  assert(pool);
  assert(cmd);
  vkFreeCommandBuffers(context->device(), pool.handle, 1, &cmd.handle);
}

void reset_command_buffer(CommandBuffer cmd) {
  assert(cmd);
  VkResult result = vkResetCommandBuffer(cmd.handle, 0);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to reset command buffer");
  }
}

void begin_command_buffer(CommandBuffer cmd) {
  VkCommandBufferInheritanceInfo inheritance{};
  inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pInheritanceInfo = &inheritance,
  };
  VkResult result = vkBeginCommandBuffer(cmd.handle, &beginInfo);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin command buffer");
  }
}

void end_command_buffer(CommandBuffer cmd) {
  VkResult result = vkEndCommandBuffer(cmd.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to end command buffer");
  }
}

} // namespace strobe::gpu::vulkan
