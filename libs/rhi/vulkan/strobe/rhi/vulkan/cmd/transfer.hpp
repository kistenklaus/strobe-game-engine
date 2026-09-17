#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/image_subresource_layers.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::rhi::vulkan {

struct BufferOffset {
  Buffer buffer{};
  VkDeviceSize offset = 0;
};

void cmd_copy_buffer(CommandBuffer cmd, BufferOffset dst, BufferOffset src,
                     VkDeviceSize size) noexcept;

void cmd_copy_buffer_to_image(CommandBuffer cmd, Image dst,
                              ImageLayout dstLayout,
                              ImageSubresourceLayers dstSubresource,
                              ivec3 dstOffset, uvec3 dstExtent,
                              BufferOffset src, uint32_t srcRowLength,
                              uint32_t srcImageHeight) noexcept;

void cmd_copy_image_to_buffer(CommandBuffer cmd, BufferOffset dst, Image src,
                              ImageLayout srcLayout,
                              ImageSubresourceLayers srcSubresource,
                              ivec3 srcOffset, uvec3 srcExtent,
                              uint32_t dstRowLength,
                              uint32_t dstImageHeight) noexcept;

void cmd_update_buffer(CommandBuffer cmd, BufferOffset dst, const void *src,
                       VkDeviceSize size) noexcept;

} // namespace strobe::rhi::vulkan
