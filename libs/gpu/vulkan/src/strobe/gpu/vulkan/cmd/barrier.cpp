#include "strobe/gpu/vulkan/cmd/barrier.hpp"

namespace strobe::gpu::vulkan {

void cmd_pipeline_barrier(CommandBuffer cmd, const PipelineBarrier &barrier) {
  Vector<VkMemoryBarrier2> memoryBarriers;
  memoryBarriers.reserve(barrier.memoryBarriers.size());

  for (const MemoryBarrier &b : barrier.memoryBarriers) {
    memoryBarriers.emplace_back(VkMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = b.srcStageMask,
        .srcAccessMask = b.srcAccessMask,
        .dstStageMask = b.dstStageMask,
        .dstAccessMask = b.dstAccessMask,
    });
  }

  Vector<VkBufferMemoryBarrier2> bufferBarriers;
  bufferBarriers.reserve(barrier.bufferBarriers.size());

  for (const BufferMemoryBarrier &b : barrier.bufferBarriers) {
    bufferBarriers.emplace_back(VkBufferMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = b.srcStageMask,
        .srcAccessMask = b.srcAccessMask,
        .dstStageMask = b.dstStageMask,
        .dstAccessMask = b.dstAccessMask,
        .srcQueueFamilyIndex = b.srcQueueFamilyIndex,
        .dstQueueFamilyIndex = b.dstQueueFamilyIndex,
        .buffer = b.buffer.handle,
        .offset = b.offset,
        .size = b.size,
    });
  }

  Vector<VkImageMemoryBarrier2> imageBarriers;
  imageBarriers.reserve(barrier.imageBarriers.size());

  for (const ImageMemoryBarrier &b : barrier.imageBarriers) {
    imageBarriers.emplace_back(VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = b.srcStageMask,
        .srcAccessMask = b.srcAccessMask,
        .dstStageMask = b.dstStageMask,
        .dstAccessMask = b.dstAccessMask,
        .oldLayout = b.oldLayout,
        .newLayout = b.newLayout,
        .srcQueueFamilyIndex = b.srcQueueFamilyIndex,
        .dstQueueFamilyIndex = b.dstQueueFamilyIndex,
        .image = b.image.handle,
        .subresourceRange = b.subresourceRange,
    });
  }

  const VkDependencyInfo dependencyInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = nullptr,
      .dependencyFlags = barrier.dependencyFlags,

      .memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size()),
      .pMemoryBarriers =
          memoryBarriers.empty() ? nullptr : memoryBarriers.data(),

      .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size()),
      .pBufferMemoryBarriers =
          bufferBarriers.empty() ? nullptr : bufferBarriers.data(),

      .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size()),
      .pImageMemoryBarriers =
          imageBarriers.empty() ? nullptr : imageBarriers.data(),
  };

  vkCmdPipelineBarrier2(cmd.handle, &dependencyInfo);
}
} // namespace strobe::gpu::vulkan
