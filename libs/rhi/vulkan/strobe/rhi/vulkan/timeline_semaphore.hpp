#pragma once

#include "strobe/rhi/vulkan/context/context.hpp"
#include <limits>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct TimelineSemaphore {
  VkSemaphore handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct TimelineSemaphoreInfo {
  uint64_t initalValue = 0;
  VkSemaphoreCreateFlags flags = 0;
};

struct Timepoint {
  TimelineSemaphore semaphore;
  uint64_t value;
};

TimelineSemaphore
create_timeline_semaphore(Context *context,
                          const TimelineSemaphoreInfo &info = {});

void destroy_timeline_semaphore(Context *context, TimelineSemaphore sem);

uint64_t get_timeline_semaphore_value(Context *context, TimelineSemaphore sem);

void signal_timeline_semaphore(Context *context, TimelineSemaphore sem,
                               uint64_t value);

bool wait_for_timeline_semaphore(
    Context *context, TimelineSemaphore sem, uint64_t value,
    uint64_t timeout = std::numeric_limits<uint64_t>::max());

} // namespace strobe::rhi::vulkan
