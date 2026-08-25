#pragma once

#include "strobe/rhi/types/cull_mode.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkCullModeFlags
to_vk_cull_mode(CullMode mode) noexcept {
  VkCullModeFlags result = 0;

  if ((mode & CullMode::front) != 0) {
    result |= VK_CULL_MODE_FRONT_BIT;
  }

  if ((mode & CullMode::back) != 0) {
    result |= VK_CULL_MODE_BACK_BIT;
  }

  return result;
}

static inline CullMode
from_vk_cull_mode(VkCullModeFlags mode) noexcept {
  CullMode result = CullMode::none;

  if ((mode & VK_CULL_MODE_FRONT_BIT) != 0) {
    result |= CullMode::front;
  }

  if ((mode & VK_CULL_MODE_BACK_BIT) != 0) {
    result |= CullMode::back;
  }

  return result;
}

} // namespace strobe::rhi
