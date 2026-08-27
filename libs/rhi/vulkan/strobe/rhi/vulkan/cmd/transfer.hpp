#pragma once

#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::rhi::vulkan {

struct BufferOffset {
  Buffer buffer{};
  VkDeviceSize offset = 0;
};

void cmd_copy_buffer(CommandBuffer cmd, BufferOffset dst, BufferOffset src,
                     VkDeviceSize size) noexcept;

void cmd_update_buffer(CommandBuffer cmd, BufferOffset dst, const void *src,
                       VkDeviceSize size) noexcept;

} // namespace strobe::rhi::vulkan
