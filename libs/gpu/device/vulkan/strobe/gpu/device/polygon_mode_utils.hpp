#pragma once

#include "strobe/gpu/device/polygon_mode.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkPolygonMode
to_vk_polygon_mode(PolygonMode mode) noexcept {
  switch (mode) {
  case PolygonMode::fill:
    return VK_POLYGON_MODE_FILL;

  case PolygonMode::line:
    return VK_POLYGON_MODE_LINE;

  case PolygonMode::point:
    return VK_POLYGON_MODE_POINT;
  }

  assert(false);
  return VK_POLYGON_MODE_FILL;
}

static inline PolygonMode
from_vk_polygon_mode(VkPolygonMode mode) noexcept {
  switch (mode) {
  case VK_POLYGON_MODE_FILL:
    return PolygonMode::fill;

  case VK_POLYGON_MODE_LINE:
    return PolygonMode::line;

  case VK_POLYGON_MODE_POINT:
    return PolygonMode::point;

  default:
    assert(false);
    return PolygonMode::fill;
  }
}

} // namespace strobe::gpu
