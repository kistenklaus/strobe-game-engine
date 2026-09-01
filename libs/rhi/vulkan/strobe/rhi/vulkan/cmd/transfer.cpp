#include "strobe/rhi/vulkan/cmd/transfer.hpp"

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
