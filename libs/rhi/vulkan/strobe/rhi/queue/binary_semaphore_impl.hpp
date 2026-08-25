#pragma once

#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"

// TODO: bad dependency
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"

namespace strobe::rhi {

struct BinarySemaphoreImpl {

  BinarySemaphoreImpl(Context context, vulkan::BinarySemaphore sem) noexcept
      : context(std::move(context)), semaphore(sem) {}

  ~BinarySemaphoreImpl() noexcept {
    vulkan::destroy_binary_semaphore(context.ctx(), semaphore);
  }

  const Context context;
  vulkan::BinarySemaphore semaphore;

  SwapchainGenerationImpl *swapchainGeneration = nullptr; // <- is set enabled special swapchain behavior
};

} // namespace strobe::rhi
