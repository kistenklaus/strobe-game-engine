#pragma once

#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct CommandBuffer {
  VkCommandBuffer handle = VK_NULL_HANDLE;
  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct CommandBufferInfo {
  CommandPool pool{};
  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
};

[[nodiscard]]
CommandBuffer alloc_command_buffer(Context *context,
                                   const CommandBufferInfo &info = {});

void free_command_buffer(Context *context, CommandPool pool,
                         CommandBuffer cmd) noexcept;

void reset_command_buffer(CommandBuffer cmd);

void begin_command_buffer(CommandBuffer cmd);

void end_command_buffer(CommandBuffer cmd);

} // namespace strobe::rhi::vulkan
