#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device_create_info.hpp"
#include "strobe/gpu/device/device_info.hpp"
#include "strobe/gpu/device/native_queue.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/debug_name.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
#include <common/TracySystem.hpp>
#include <limits>
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
            .debug_utils = createInfo.debug_utils ? vulkan::feature::required
                                                  : vulkan::feature::disable,
            .surface = vulkan::feature::required,
            .swapchain = vulkan::feature::required,
            .timeline_semaphore = vulkan::feature::required,
            .queue_count = NATIVE_QUEUE_COUNT,
            .pQueues = NATIVE_QUEUE_DESC.data()}) {
    // query the native queues
    for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
      nativeQueues[i].queue = context.get()->queue(i);
      nativeQueues[i].timeline =
          vulkan::create_timeline_semaphore(context.get(), {.initalValue = 0});
    }
    gc_thread = std::jthread(
        [](std::stop_token st, DeviceImpl *impl) { impl->gc_thread_main(st); },
        this);
  }
  DeviceImpl(const DeviceImpl &) = delete;
  DeviceImpl(DeviceImpl &&) = delete;
  DeviceImpl &operator=(const DeviceImpl &) = delete;
  DeviceImpl &operator=(DeviceImpl &&) = delete;
  ~DeviceImpl() {
    // gc_thread's stop callback wakes vkWaitSemaphores.
    gc_thread.request_stop();
    if (gc_thread.joinable()) {
      gc_thread.join();
    }
    for (NativeQueue &queue : nativeQueues) {
      if (queue.timeline) {
        vulkan::destroy_timeline_semaphore(context.get(), queue.timeline);

        queue.timeline = {};
      }
    }
  }

  void gc_thread_main(std::stop_token st) {
    tracy::SetThreadName("strobe-gpu-gc");
    constexpr uint32_t WAKE_INDEX = NATIVE_QUEUE_COUNT;
    constexpr uint32_t SEMAPHORE_COUNT = NATIVE_QUEUE_COUNT + 1;

    std::array<VkSemaphore, SEMAPHORE_COUNT> semaphores{};
    std::array<uint64_t, SEMAPHORE_COUNT> waitValues{};
    std::array<uint64_t, NATIVE_QUEUE_COUNT> completedValues{};

    for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
      semaphores[i] = nativeQueues[i].timeline.handle;
      waitValues[i] = 1;
    }

    vulkan::TimelineSemaphore wakeSemaphore =
        vulkan::create_timeline_semaphore(context.get(), {.initalValue = 0});
    vulkan::set_debug_name(context.get(), wakeSemaphore, "gc-thread-wake");

    semaphores[WAKE_INDEX] = wakeSemaphore.handle;
    waitValues[WAKE_INDEX] = 1;

    {
      std::stop_callback stopCallback{
          st, [device = context.get()->device(), wakeSemaphore]() noexcept {
            VkSemaphoreSignalInfo signalInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                .pNext = nullptr,
                .semaphore = wakeSemaphore.handle,
                .value = 1,
            };
            {
              ZoneScopedN("vkSignalSemaphore");
              (void)vkSignalSemaphore(device, &signalInfo);
            }
          }};

      while (!st.stop_requested()) {
        VkSemaphoreWaitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = VK_SEMAPHORE_WAIT_ANY_BIT,
            .semaphoreCount = SEMAPHORE_COUNT,
            .pSemaphores = semaphores.data(),
            .pValues = waitValues.data(),
        };

        VkResult result;
        {
          ZoneScopedN("vkWaitSemaphores");
          result = vkWaitSemaphores(context.get()->device(), &waitInfo,
                                    std::numeric_limits<uint64_t>::max());
        }

        if (result != VK_SUCCESS) {
          break;
        }

        if (st.stop_requested()) {
          break;
        }

        for (uint32_t i = 0; i < NATIVE_QUEUE_COUNT; ++i) {
          uint64_t completed = 0;

          {
            ZoneScopedN("vkGetSemaphoreCounterValue");
            result = vkGetSemaphoreCounterValue(context.get()->device(),
                                                semaphores[i], &completed);
          }

          if (result != VK_SUCCESS) {
            break;
          }

          if (completed > completedValues[i]) {
            completedValues[i] = completed;

            nativeQueues[i].collect_submissions(completed);

            // Wait until this queue makes further progress.
            waitValues[i] = completed + 1;
          }
        }

        if (result != VK_SUCCESS) {
          break;
        }
      }
    }

    // stopCallback is guaranteed gone before destroying its semaphore.
    vulkan::destroy_timeline_semaphore(context.get(), wakeSemaphore);
  }

  Context context;
  std::array<NativeQueue, NATIVE_QUEUE_COUNT> nativeQueues;
  std::jthread gc_thread;
  DeviceInfo info;
};

} // namespace strobe::gpu
