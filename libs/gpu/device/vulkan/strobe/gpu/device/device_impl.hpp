#pragma once

#include "strobe/gpu/device/device_create_info.hpp"
#include "strobe/gpu/device/device_info.hpp"
#include "strobe/gpu/device/native_queue.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
#include <atomic>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static const std::array<vulkan::QueueDescription, 4> NATIVE_QUEUE_DESC{
    // Main graphics queue.
    vulkan::QueueDescription{
        .require = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
                   VK_QUEUE_TRANSFER_BIT,
        .available = vulkan::feature::required,
    },
    // Presentation queue.
    vulkan::QueueDescription{
        .present = vulkan::feature::required,
        .available = vulkan::feature::required,
    },

    // Prefer a genuinely transfer-only queue.
    vulkan::QueueDescription{
        .require = VK_QUEUE_TRANSFER_BIT,
        .exclude = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
        .available = vulkan::feature::optional,
    },

    // Prefer an async-compute queue without graphics support.
    vulkan::QueueDescription{
        .require = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        .exclude = VK_QUEUE_GRAPHICS_BIT,
        .available = vulkan::feature::optional,
    },
};
static constexpr uint32_t NATIVE_QUEUE_COUNT = NATIVE_QUEUE_DESC.size();

struct DeviceImpl {
  DeviceImpl(const DeviceCreateInfo &createInfo)
      : context(vulkan::ContextCreateInfo{
            .debug_utils = createInfo.debug_utils ? vulkan::feature::disable
                                                  : vulkan::feature::required,
            .surface = vulkan::feature::required,
            .swapchain = vulkan::feature::required,
            .timeline_semaphore = vulkan::feature::required,
            .queue_count = NATIVE_QUEUE_COUNT,
            .pQueues = NATIVE_QUEUE_DESC.data()}) {
    // query the native queues
    for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
      nativeQueues[i].queue = context.queue(i);
      nativeQueues[i].timeline =
          vulkan::create_timeline_semaphore(&context, {.initalValue = 0});
      nativeQueues[i].nextTimelineValue.store(0, std::memory_order_relaxed);
    }
  }

  vulkan::Context context;
  std::array<NativeQueue, NATIVE_QUEUE_COUNT> nativeQueues;

  DeviceInfo info;
};

} // namespace strobe::gpu
