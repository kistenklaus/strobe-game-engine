#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

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
  {
    ZoneScopedN("vkAllocateCommandBuffers");
    VkResult result =
        vkAllocateCommandBuffers(context->device(), &allocInfo, &cmd.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to allocate command buffer");
    }
  }
  return cmd;
}

void free_command_buffer(Context *context, CommandPool pool,
                         CommandBuffer cmd) noexcept {
  assert(context != nullptr);
  assert(pool);
  assert(cmd);
  ZoneScopedN("vkFreeCommandBuffers");
  vkFreeCommandBuffers(context->device(), pool.handle, 1, &cmd.handle);
}

void reset_command_buffer(CommandBuffer cmd) {
  assert(cmd);
  {
    ZoneScopedN("vkResetCommandBuffer");
    VkResult result = vkResetCommandBuffer(cmd.handle, 0);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to reset command buffer");
    }
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
  {
    ZoneScopedN("vkBeginCommandBuffer");
    VkResult result = vkBeginCommandBuffer(cmd.handle, &beginInfo);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to begin command buffer");
    }
  }
}

void end_command_buffer(CommandBuffer cmd) {
  {
    ZoneScopedN("vkEndCommandBuffer");
    VkResult result = vkEndCommandBuffer(cmd.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to end command buffer");
    }
  }
}

} // namespace strobe::rhi::vulkan
