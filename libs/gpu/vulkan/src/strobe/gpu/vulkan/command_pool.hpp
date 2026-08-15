#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct CommandPool {
  VkCommandPool handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct CommandPoolInfo {
  Queue queue{};
  VkCommandPoolCreateFlags flags = 0;
};

CommandPool create_command_pool(Context *context,
                                const CommandPoolInfo &info = {});


void destroy_command_pool(Context *context, CommandPool cmdpool) noexcept;

void reset_command_pool(Context *context, CommandPool cmdpool,
                        VkCommandPoolCreateFlags flags = 0);

} // namespace strobe::gpu::vulkan
