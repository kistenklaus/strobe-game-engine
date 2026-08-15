#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
namespace strobe::gpu {

struct TimelineSemaphoreImpl {

  explicit TimelineSemaphoreImpl(Context context,
                                 vulkan::TimelineSemaphore sem) noexcept
      : context(std::move(context)), semaphore(sem) {}

  ~TimelineSemaphoreImpl() noexcept {
    vulkan::destroy_timeline_semaphore(context.get(), semaphore);
  }

  Context context;
  vulkan::TimelineSemaphore semaphore;
};
} // namespace strobe::gpu
