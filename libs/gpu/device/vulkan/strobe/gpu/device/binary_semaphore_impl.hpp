#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include "strobe/gpu/vulkan/binary_semaphore.hpp"
namespace strobe::gpu {

struct BinarySemaphoreImpl {

  BinarySemaphoreImpl(Context context, vulkan::BinarySemaphore sem) noexcept
      : context(std::move(context)), semaphore(sem) {}

  ~BinarySemaphoreImpl() noexcept {
    vulkan::destroy_binary_semaphore(context.get(), semaphore);
  }

  const Context context;
  vulkan::BinarySemaphore semaphore;

  SwapchainGenerationImpl *swapchainGeneration = nullptr; // <- is set enabled special swapchain behavior
};

} // namespace strobe::gpu
