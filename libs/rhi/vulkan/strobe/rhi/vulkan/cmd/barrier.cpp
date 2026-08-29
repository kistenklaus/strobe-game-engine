#include "strobe/rhi/vulkan/cmd/barrier.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/utils/access_utils.hpp"
#include "strobe/rhi/utils/image_aspect_utils.hpp"
#include "strobe/rhi/utils/image_layout_utils.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void cmd_pipeline_barrier(
    CommandBuffer cmd, span<const VkMemoryBarrier2> memoryBarriers,
    span<const VkBufferMemoryBarrier2> bufferMemoryBarriers,
    span<const VkImageMemoryBarrier2> imageMemoryBarriers,
    VkDependencyFlags dependencyFlags) noexcept {
  VkDependencyInfo depInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = nullptr,
      .dependencyFlags = dependencyFlags,
      .memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size()),
      .pMemoryBarriers = memoryBarriers.data(),
      .bufferMemoryBarrierCount =
          static_cast<uint32_t>(bufferMemoryBarriers.size()),
      .pBufferMemoryBarriers = bufferMemoryBarriers.data(),
      .imageMemoryBarrierCount =
          static_cast<uint32_t>(imageMemoryBarriers.size()),
      .pImageMemoryBarriers = imageMemoryBarriers.data(),
  };
  {
    ZoneScopedN("vkCmdPipelineBarrier2");
    vkCmdPipelineBarrier2(cmd.handle, &depInfo);
  }
}
void cmd_memory_barrier(CommandBuffer cmd,
                        span<const MemoryBarrier> memoryBarriers,
                        VkDependencyFlags dependencyFlags) noexcept {
  SmallVector<VkMemoryBarrier2, 4> barriers{memoryBarriers.size()};
  for (uint32_t i = 0; i < barriers.size(); ++i) {
    barriers[i] = VkMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = to_vk_pipeline_stage(memoryBarriers[i].srcStage),
        .srcAccessMask = to_vk_access(memoryBarriers[i].srcAccess),
        .dstStageMask = to_vk_pipeline_stage(memoryBarriers[i].dstStage),
        .dstAccessMask = to_vk_access(memoryBarriers[i].dstAccess),
    };
  }
  cmd_pipeline_barrier(cmd, barriers, {}, {}, dependencyFlags);
}
void cmd_memory_barrier(CommandBuffer cmd, MemoryBarrier memoryBarrier,
                        VkDependencyFlags dependencyFlags) noexcept {
  VkMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      .srcStageMask = to_vk_pipeline_stage(memoryBarrier.srcStage),
      .srcAccessMask = to_vk_access(memoryBarrier.srcAccess),
      .dstStageMask = to_vk_pipeline_stage(memoryBarrier.dstStage),
      .dstAccessMask = to_vk_access(memoryBarrier.dstAccess),
  };
  cmd_pipeline_barrier(cmd, &barrier, {}, {}, dependencyFlags);
}

struct SyncScope {
  VkPipelineStageFlags2 stageMask;
  VkAccessFlags2 accessMask;
};

constexpr SyncScope layout_scope(ImageLayout layout) {
  switch (layout) {
  case ImageLayout::undefined:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_NONE,
        .accessMask = VK_ACCESS_2_NONE,
    };

  case ImageLayout::general:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .accessMask =
            VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
    };

  case ImageLayout::read_only:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .accessMask = VK_ACCESS_2_MEMORY_READ_BIT,
    };

  case ImageLayout::attachment:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT |
                     VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

        .accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
                      VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                      VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

  case ImageLayout::transfer_src:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .accessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
    };

  case ImageLayout::transfer_dst:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
    };

  case ImageLayout::present:
    return {
        .stageMask = VK_PIPELINE_STAGE_2_NONE,
        .accessMask = VK_ACCESS_2_NONE,
    };
  }
  std::unreachable();
}

void cmd_transition_image(CommandBuffer cmd, Image image,
                          ImageSubresourceRange subresource, ImageLayout src,
                          ImageLayout dst) noexcept {

  const VkPipelineStageFlags2 srcStage = layout_scope(src).stageMask;
  const VkAccessFlags2 srcAccess = layout_scope(src).accessMask;

  const VkPipelineStageFlags2 dstStage = layout_scope(dst).stageMask;
  const VkAccessFlags2 dstAccess = layout_scope(dst).accessMask;

  const VkImageMemoryBarrier2 transition{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .pNext = nullptr,
      .srcStageMask = srcStage,
      .srcAccessMask = srcAccess,
      .dstStageMask = dstStage,
      .dstAccessMask = dstAccess,
      .oldLayout = to_vk_image_layout(src),
      .newLayout = to_vk_image_layout(dst),
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image.handle,
      .subresourceRange =
          VkImageSubresourceRange{
              .aspectMask = to_vk_image_aspect(subresource.aspect),
              .baseMipLevel = subresource.baseMipLevel,
              .levelCount = subresource.levelCount,
              .baseArrayLayer = subresource.baseArrayLayer,
              .layerCount = subresource.layerCount,
          },

  };
  cmd_pipeline_barrier(cmd, {}, {}, &transition, 0);
}

} // namespace strobe::rhi::vulkan
