#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

CommandPool create_command_pool(Context *context, const CommandPoolInfo &info) {

  VkCommandPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .queueFamilyIndex = info.queueFamily,
  };
  CommandPool cmdpool;
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCreateCommandPool");
#endif
    VkResult result =
        vkCreateCommandPool(context->device(), &createInfo,
                            context->driver_alloc(), &cmdpool.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create command buffer");
    }
  }
  return cmdpool;
}

void destroy_command_pool(Context *context, CommandPool cmdpool) noexcept {
  assert(context);
  assert(cmdpool);
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkDestroyCommandPool");
#endif
    vkDestroyCommandPool(context->device(), cmdpool.handle,
                         context->driver_alloc());
  }
}

void reset_command_pool(Context *context, CommandPool cmdpool,
                        VkCommandPoolResetFlags flags) {
  assert(context != nullptr);
  assert(cmdpool);
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkResetCommandPool");
#endif
    VkResult result =
        vkResetCommandPool(context->device(), cmdpool.handle, flags);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to reset command pool");
    }
  }
}

} // namespace strobe::rhi::vulkan
