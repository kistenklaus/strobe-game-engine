#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/gpu/device/access_utils.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/command_buffer_handle_alloc.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_aspect_utils.hpp"
#include "strobe/gpu/device/image_impl.hpp"
#include "strobe/gpu/device/image_layout_utils.hpp"
#include "strobe/gpu/device/pipeline_stage_utils.hpp"
#include "strobe/gpu/device/queue_impl.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

using handle_alloc = cmd_buf_handle_allocator_ref;

namespace {

void unpin_command_buffer(void *h) noexcept {
  if (h == nullptr) {
    return;
  }

  using control_block = handle_control_block<CommandBufferImpl, handle_alloc>;
  using allocator_traits = AllocatorTraits<handle_alloc>;

  auto *block = static_cast<control_block *>(h);

  if (block->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }

  // Keep CommandPoolImpl alive while destroying the CommandBufferImpl
  // and returning this control block to its allocator.
  //
  // Do this only on the final release, so ordinary CommandBuffer copies
  // don't cause extra CommandPool refcount traffic.
  CommandPool keepAlive = block->value.pool;

  // The allocator object itself is part of the control block.
  // Move it out before destroying the block.
  handle_alloc alloc = std::move(block->alloc);

  std::destroy_at(block);

  allocator_traits::template deallocate<control_block>(alloc, block);

  // keepAlive dies here, after the allocator has been used.
}

} // namespace

CommandBuffer::CommandBuffer(const CommandBuffer &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl, handle_alloc>(m_handle);
  }
}

CommandBuffer::CommandBuffer(CommandBuffer &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

CommandBuffer &CommandBuffer::operator=(const CommandBuffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl, handle_alloc>(o.m_handle);
  }
  unpin_command_buffer(m_handle);
  m_handle = o.m_handle;
  return *this;
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_command_buffer(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

CommandBuffer::~CommandBuffer() noexcept {
  unpin_command_buffer(m_handle);
}

void CommandBuffer::begin() {
  ZoneScopedN("CommandBuffer::begin");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);
  vulkan::begin_command_buffer(impl->cmd);
}

void CommandBuffer::end() {
  ZoneScopedN("CommandBuffer::end");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);
  vulkan::end_command_buffer(impl->cmd);
}

void CommandBuffer::barrier(const Barrier &barrier) {
  ZoneScopedN("CommandBuffer::barrier");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);

  static constexpr size_t SCRATCH_SIZE = 1024;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::gpu::allocator, SCRATCH_SIZE>;
  scratch_allocator scratch{};
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  Vector<VkMemoryBarrier2, scratch_allocator_ref> memoryBarriers{
      barrier.memoryBarriers.size(), &scratch};
  for (uint32_t i = 0; i < memoryBarriers.size(); ++i) {
    const auto &src = barrier.memoryBarriers[i];
    memoryBarriers[i] = VkMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = to_vk_pipeline_stage(src.srcStage),
        .srcAccessMask = to_vk_access(src.srcAccess),
        .dstStageMask = to_vk_pipeline_stage(src.dstStage),
        .dstAccessMask = to_vk_access(src.dstAccess),
    };
  }

  Vector<VkBufferMemoryBarrier2, scratch_allocator_ref> bufferBarriers{
      barrier.bufferBarriers.size(), &scratch};
  for (uint32_t i = 0; i < bufferBarriers.size(); ++i) {
    const auto &src = barrier.bufferBarriers[i];
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    if (src.srcQueue && src.dstQueue) {
      srcQueueFamily = void_handle_ptr<QueueImpl>(src.srcQueue.m_handle)
                           ->native->queue.family;
      dstQueueFamily = void_handle_ptr<QueueImpl>(src.dstQueue.m_handle)
                           ->native->queue.family;
    }
    uint64_t size = src.size;
    if (size == 0 || size > src.buffer.size()) {
      size = src.buffer.size();
    }
    bufferBarriers[i] = VkBufferMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = to_vk_pipeline_stage(src.srcStage),
        .srcAccessMask = to_vk_access(src.srcAccess),
        .dstStageMask = to_vk_pipeline_stage(src.dstStage),
        .dstAccessMask = to_vk_access(src.dstAccess),
        .srcQueueFamilyIndex = srcQueueFamily,
        .dstQueueFamilyIndex = dstQueueFamily,
        .buffer =
            void_handle_ptr<BufferImpl>(src.buffer.m_handle)->buffer.handle,
        .offset = src.offset,
        .size = size,
    };
  }

  Vector<VkImageMemoryBarrier2, scratch_allocator_ref> imageBarriers{
      barrier.imageBarriers.size(), &scratch};
  for (uint32_t i = 0; i < imageBarriers.size(); ++i) {
    const auto &src = barrier.imageBarriers[i];
    uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    if (src.srcQueue && src.dstQueue) {
      srcQueueFamily = void_handle_ptr<QueueImpl>(src.srcQueue.m_handle)
                           ->native->queue.family;
      dstQueueFamily = void_handle_ptr<QueueImpl>(src.dstQueue.m_handle)
                           ->native->queue.family;
    }
    imageBarriers[i] = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = to_vk_pipeline_stage(src.srcStage),
        .srcAccessMask = to_vk_access(src.srcAccess),
        .dstStageMask = to_vk_pipeline_stage(src.dstStage),
        .dstAccessMask = to_vk_access(src.dstAccess),
        .oldLayout = to_vk_image_layout(src.srcLayout),
        .newLayout = to_vk_image_layout(src.dstLayout),
        .srcQueueFamilyIndex = srcQueueFamily,
        .dstQueueFamilyIndex = dstQueueFamily,
        .image = void_handle_ptr<ImageImpl>(src.image.m_handle)->image.handle,
        .subresourceRange =
            {
                .aspectMask = to_vk_image_aspect(src.range.aspect),
                .baseMipLevel = src.range.baseMipLevel,
                .levelCount = src.range.levelCount == REMAINING_MIP_LEVELS
                                  ? VK_REMAINING_MIP_LEVELS
                                  : src.range.levelCount,
                .baseArrayLayer = src.range.baseArrayLayer,
                .layerCount = src.range.layerCount == REMAINING_ARRAY_LAYERS
                                  ? VK_REMAINING_ARRAY_LAYERS
                                  : src.range.layerCount,
            },
    };
  }

  VkDependencyInfo dependencyInfo{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .pNext = nullptr,
      .dependencyFlags = 0,
      .memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size()),
      .pMemoryBarriers = memoryBarriers.data(),
      .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size()),
      .pBufferMemoryBarriers = bufferBarriers.data(),
      .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size()),
      .pImageMemoryBarriers = imageBarriers.data(),
  };
  {
    ZoneScopedN("vkCmdPipelineBarrier2");
    vkCmdPipelineBarrier2(impl->cmd.handle, &dependencyInfo);
  }
}

} // namespace strobe::gpu
