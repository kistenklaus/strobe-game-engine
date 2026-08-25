#pragma once

#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
namespace strobe::rhi {

struct TimelineSemaphoreImpl {

  explicit TimelineSemaphoreImpl(Context context,
                                 vulkan::TimelineSemaphore sem) noexcept
      : context(std::move(context)), semaphore(sem) {}

  ~TimelineSemaphoreImpl() noexcept {
    vulkan::destroy_timeline_semaphore(context.ctx(), semaphore);
  }

  Context context;
  vulkan::TimelineSemaphore semaphore;
};
} // namespace strobe::rhi
