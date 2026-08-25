#pragma once

#include "strobe/rhi/types/primitive_topology.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkPrimitiveTopology
to_vk_primitive_topology(PrimitiveTopology topology) noexcept {
  switch (topology) {
  case PrimitiveTopology::point_list:
    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

  case PrimitiveTopology::line_list:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

  case PrimitiveTopology::line_strip:
    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

  case PrimitiveTopology::triangle_list:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  case PrimitiveTopology::triangle_strip:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  case PrimitiveTopology::triangle_fan:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

  case PrimitiveTopology::line_list_with_adjacency:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

  case PrimitiveTopology::line_strip_with_adjacency:
    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;

  case PrimitiveTopology::triangle_list_with_adjacency:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;

  case PrimitiveTopology::triangle_strip_with_adjacency:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;

  case PrimitiveTopology::patch_list:
    return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
  }

  assert(false);
  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

static inline PrimitiveTopology
from_vk_primitive_topology(VkPrimitiveTopology topology) noexcept {
  switch (topology) {
  case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
    return PrimitiveTopology::point_list;

  case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
    return PrimitiveTopology::line_list;

  case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
    return PrimitiveTopology::line_strip;

  case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
    return PrimitiveTopology::triangle_list;

  case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
    return PrimitiveTopology::triangle_strip;

  case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
    return PrimitiveTopology::triangle_fan;

  case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
    return PrimitiveTopology::line_list_with_adjacency;

  case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
    return PrimitiveTopology::line_strip_with_adjacency;

  case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
    return PrimitiveTopology::triangle_list_with_adjacency;

  case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
    return PrimitiveTopology::triangle_strip_with_adjacency;

  case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
    return PrimitiveTopology::patch_list;

  default:
    assert(false);
    return PrimitiveTopology::triangle_list;
  }
}

} // namespace strobe::rhi
