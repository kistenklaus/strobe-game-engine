#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"

namespace strobe::rhi {

struct BinarySemaphoreImpl {

  BinarySemaphoreImpl(Context context, vulkan::BinarySemaphore sem) noexcept
      : context(std::move(context)), semaphore(sem) {}

  ~BinarySemaphoreImpl() noexcept {
    vulkan::destroy_binary_semaphore(context.ctx(), semaphore);
  }

  const Context context;
  const vulkan::BinarySemaphore semaphore;

  // NOTE: if anything a type erased destructor.
  // SwapchainGenerationImpl *swapchainGeneration = nullptr; // <- is set
  // enabled special swapchain behavior
};

} // namespace strobe::rhi
