#pragma once

#include "strobe/rhi/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct BinarySemaphore {
  VkSemaphore handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct SemaphoreInfo {
  VkSemaphoreCreateFlags flags = 0;
};

BinarySemaphore create_binary_semaphore(Context *context, const SemaphoreInfo& info = {});

void destroy_binary_semaphore(Context *context, BinarySemaphore sem) noexcept;

} // namespace strobe::rhi::vulkan
