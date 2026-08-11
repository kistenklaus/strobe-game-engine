#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/image.hpp"
namespace strobe::gpu::vulkan {

struct MemoryBarrier {
  VkPipelineStageFlags2 srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 srcAccessMask = VK_ACCESS_2_NONE;

  VkPipelineStageFlags2 dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 dstAccessMask = VK_ACCESS_2_NONE;
};

struct BufferMemoryBarrier {
  VkPipelineStageFlags2 srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 srcAccessMask = VK_ACCESS_2_NONE;

  VkPipelineStageFlags2 dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 dstAccessMask = VK_ACCESS_2_NONE;

  uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  Buffer buffer = {};
  VkDeviceSize offset = 0;
  VkDeviceSize size = VK_WHOLE_SIZE;
};

struct ImageMemoryBarrier {
  VkPipelineStageFlags2 srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 srcAccessMask = VK_ACCESS_2_NONE;

  VkPipelineStageFlags2 dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 dstAccessMask = VK_ACCESS_2_NONE;

  VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  Image image = {};

  VkImageSubresourceRange subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };
};

struct PipelineBarrier {
  VkDependencyFlags dependencyFlags = 0;

  span<const MemoryBarrier> memoryBarriers = {};
  span<const BufferMemoryBarrier> bufferBarriers = {};
  span<const ImageMemoryBarrier> imageBarriers = {};
};

void cmd_pipeline_barrier(CommandBuffer cmd, const PipelineBarrier &barrier);

} // namespace strobe::gpu::vulkan
