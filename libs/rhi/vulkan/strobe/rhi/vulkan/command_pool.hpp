#pragma once

#include "strobe/rhi/vulkan/context/context.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct CommandPool {
  VkCommandPool handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct CommandPoolInfo {
  uint32_t queueFamily = 0;
  VkCommandPoolCreateFlags flags = 0;
};

CommandPool create_command_pool(Context *context,
                                const CommandPoolInfo &info = {});


void destroy_command_pool(Context *context, CommandPool cmdpool) noexcept;

void reset_command_pool(Context *context, CommandPool cmdpool,
                        VkCommandPoolCreateFlags flags = 0);

} // namespace strobe::rhi::vulkan
