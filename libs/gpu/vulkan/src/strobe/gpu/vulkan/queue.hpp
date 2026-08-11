#pragma once

#include "strobe/gpu/vulkan/binary_semaphore.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/fence.hpp"
#include "strobe/gpu/vulkan/queue_type.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct TimelineSemaphoreSubmitInfo {
  TimelineSemaphore semaphore = {};
  uint64_t value = 0;
  VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
};

struct BinarySemaphoreSubmitInfo {
  BinarySemaphore semaphore = {};
  VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
};

struct SubmitInfo {
  span<const TimelineSemaphoreSubmitInfo> waitTimelineSemaphores = {};
  span<const BinarySemaphoreSubmitInfo> waitBinarySemaphores = {};
  span<const CommandBuffer> command_buffers = {};
  span<const TimelineSemaphoreSubmitInfo> signalTimelineSemaphores = {};
  span<const BinarySemaphoreSubmitInfo> signalBinarySemaphores = {};
  Fence fence = {};
};

struct PresentInfo {
  span<const BinarySemaphore> waitBinarySemaphores = {};
};

enum class PresentStatus {
  success,
  suboptimal,
  out_of_date,
};

// requires synchronization2
void queue_submit(Queue queue, const SubmitInfo &info);

void wait_queue_idle(Queue queue);

PresentStatus queue_present(Queue queue, Swapchain swapchain,
                            uint32_t imageIndex, const PresentInfo &info = {});

} // namespace strobe::gpu::vulkan
