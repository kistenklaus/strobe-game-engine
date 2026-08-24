#pragma once

#include "strobe/gpu/device/geometry_flags.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkGeometryFlagsKHR
to_vk_geometry_flags(GeometryFlags flags) noexcept {
  VkGeometryFlagsKHR result = 0;

  if ((flags & GeometryFlags::opaque) != 0) {
    result |= VK_GEOMETRY_OPAQUE_BIT_KHR;
  }

  if ((flags & GeometryFlags::no_duplicate_any_hit) != 0) {
    result |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
  }

  return result;
}

static inline GeometryFlags
from_vk_geometry_flags(VkGeometryFlagsKHR flags) noexcept {
  GeometryFlags result = GeometryFlags::none;

  if ((flags & VK_GEOMETRY_OPAQUE_BIT_KHR) != 0) {
    result |= GeometryFlags::opaque;
  }

  if ((flags & VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR) != 0) {
    result |= GeometryFlags::no_duplicate_any_hit;
  }

  return result;
}

} // namespace strobe::gpu
