#pragma once

#include "strobe/rhi/types/geometry_type.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkGeometryTypeKHR
to_vk_geometry_type(GeometryType type) noexcept {
  switch (type) {
  case GeometryType::triangles:
    return VK_GEOMETRY_TYPE_TRIANGLES_KHR;

  case GeometryType::aabbs:
    return VK_GEOMETRY_TYPE_AABBS_KHR;

  case GeometryType::instances:
    return VK_GEOMETRY_TYPE_INSTANCES_KHR;
  }

  assert(false);
  return VK_GEOMETRY_TYPE_TRIANGLES_KHR;
}

static inline GeometryType
from_vk_geometry_type(VkGeometryTypeKHR type) noexcept {
  switch (type) {
  case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
    return GeometryType::triangles;

  case VK_GEOMETRY_TYPE_AABBS_KHR:
    return GeometryType::aabbs;

  case VK_GEOMETRY_TYPE_INSTANCES_KHR:
    return GeometryType::instances;

  default:
    assert(false);
    return GeometryType::triangles;
  }
}

} // namespace strobe::rhi
