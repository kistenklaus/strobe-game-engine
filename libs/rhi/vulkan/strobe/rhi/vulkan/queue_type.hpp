#pragma once

#include <limits>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct Queue {
  VkQueue handle = VK_NULL_HANDLE;
  uint32_t family = std::numeric_limits<uint32_t>::max();

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

} // namespace strobe::rhi::vulkan
