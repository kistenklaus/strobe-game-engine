#include "strobe/gpu/vulkan/command_pool.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

CommandPool create_command_pool(Context *context, const CommandPoolInfo &info) {

  VkCommandPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .queueFamilyIndex = info.queue.family,
  };
  CommandPool cmdpool;
  VkResult result = vkCreateCommandPool(
      context->device(), &createInfo, context->driver_alloc(), &cmdpool.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command buffer");
  }
  return cmdpool;
}

void destroy_command_pool(Context *context, CommandPool cmdpool) noexcept {
  assert(context);
  assert(cmdpool);
  vkDestroyCommandPool(context->device(), cmdpool.handle,
                       context->driver_alloc());
}

void reset_command_pool(Context *context, CommandPool cmdpool,
                        VkCommandPoolResetFlags flags) {
  assert(context != nullptr);
  assert(cmdpool);
  VkResult result = vkResetCommandPool(context->device(), cmdpool.handle, flags);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to reset command pool");
  }
}

} // namespace strobe::gpu::vulkan
