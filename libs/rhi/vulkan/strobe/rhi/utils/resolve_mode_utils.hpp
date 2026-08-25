#pragma once

#include "strobe/rhi/types/resolve_mode.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkResolveModeFlagBits
to_vk_resolve_mode(ResolveMode mode) noexcept {
  switch (mode) {
  case ResolveMode::none:
    return VK_RESOLVE_MODE_NONE;

  case ResolveMode::sample_zero:
    return VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;

  case ResolveMode::average:
    return VK_RESOLVE_MODE_AVERAGE_BIT;

  case ResolveMode::min:
    return VK_RESOLVE_MODE_MIN_BIT;

  case ResolveMode::max:
    return VK_RESOLVE_MODE_MAX_BIT;
  }

  assert(false);
  return VK_RESOLVE_MODE_NONE;
}

static inline ResolveMode
from_vk_resolve_mode(VkResolveModeFlagBits mode) noexcept {
  switch (mode) {
  case VK_RESOLVE_MODE_NONE:
    return ResolveMode::none;

  case VK_RESOLVE_MODE_SAMPLE_ZERO_BIT:
    return ResolveMode::sample_zero;

  case VK_RESOLVE_MODE_AVERAGE_BIT:
    return ResolveMode::average;

  case VK_RESOLVE_MODE_MIN_BIT:
    return ResolveMode::min;

  case VK_RESOLVE_MODE_MAX_BIT:
    return ResolveMode::max;

  default:
    assert(false);
    return ResolveMode::none;
  }
}

} // namespace strobe::rhi
