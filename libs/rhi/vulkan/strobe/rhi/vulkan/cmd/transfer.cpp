#include "strobe/rhi/vulkan/cmd/transfer.hpp"
#include "strobe/rhi/types/image_subresource_layers.hpp"
#include "strobe/rhi/utils/image_aspect_utils.hpp"
#include "strobe/rhi/utils/image_layout_utils.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void cmd_copy_buffer(CommandBuffer cmd, BufferOffset dst, BufferOffset src,
                     VkDeviceSize size) noexcept {
  VkBufferCopy copy{
      .srcOffset = src.offset,
      .dstOffset = dst.offset,
      .size = size,
  };
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCmdCopyBuffer");
#endif
    vkCmdCopyBuffer(cmd.handle, src.buffer.handle, dst.buffer.handle, 1, &copy);
  }
}
void cmd_copy_buffer_to_image(CommandBuffer cmd, Image dst,
                              ImageLayout dstLayout,
                              ImageSubresourceLayers dstSubresource,
                              ivec3 dstOffset, uvec3 dstExtent,
                              BufferOffset src, uint32_t srcRowLength,
                              uint32_t srcImageHeight) noexcept {

  VkBufferImageCopy copy{
      .bufferOffset = src.offset,
      .bufferRowLength = srcRowLength,
      .bufferImageHeight = srcImageHeight,
      .imageSubresource =
          {
              .aspectMask = to_vk_image_aspect(dstSubresource.aspect),
              .mipLevel = dstSubresource.mipLevel,
              .baseArrayLayer = dstSubresource.baseArrayLayer,
              .layerCount = dstSubresource.layerCount,
          },
      .imageOffset =
          {
              .x = dstOffset.x(),
              .y = dstOffset.y(),
              .z = dstOffset.z(),
          },
      .imageExtent =
          {
              .width = dstExtent.x(),
              .height = dstExtent.y(),
              .depth = dstExtent.z(),
          },
  };

#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkCmdCopyBufferToImage");
#endif
  vkCmdCopyBufferToImage(cmd.handle, src.buffer.handle, dst.handle,
                         to_vk_image_layout(dstLayout), 1, &copy);
}

void cmd_copy_image_to_buffer(CommandBuffer cmd, BufferOffset dst, Image src,
                              ImageLayout srcLayout,
                              ImageSubresourceLayers srcSubresource,
                              ivec3 srcOffset, uvec3 srcExtent,
                              uint32_t dstRowLength,
                              uint32_t dstImageHeight) noexcept {

  VkBufferImageCopy copy{
      .bufferOffset = dst.offset,
      .bufferRowLength = dstRowLength,
      .bufferImageHeight = dstImageHeight,
      .imageSubresource =
          {
              .aspectMask = to_vk_image_aspect(srcSubresource.aspect),
              .mipLevel = srcSubresource.mipLevel,
              .baseArrayLayer = srcSubresource.baseArrayLayer,
              .layerCount = srcSubresource.layerCount,
          },
      .imageOffset =
          {
              .x = srcOffset.x(),
              .y = srcOffset.y(),
              .z = srcOffset.z(),
          },
      .imageExtent =
          {
              .width = srcExtent.x(),
              .height = srcExtent.y(),
              .depth = srcExtent.z(),
          },
  };

#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkCmdCopyImageToBuffer");
#endif
  vkCmdCopyImageToBuffer(cmd.handle, src.handle, to_vk_image_layout(srcLayout),
                         dst.buffer.handle, 1, &copy);
}

void cmd_update_buffer(CommandBuffer cmd, BufferOffset dst, const void *src,
                       VkDeviceSize size) noexcept {
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCmdUpdateBuffer");
#endif
    vkCmdUpdateBuffer(cmd.handle, dst.buffer.handle, dst.offset, size, src);
  }
}

} // namespace strobe::rhi::vulkan
