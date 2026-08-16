#pragma once

#include "strobe/gpu/device/vertex_input_rate.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkVertexInputRate
to_vk_vertex_input_rate(VertexInputRate rate) noexcept {
  switch (rate) {
  case VertexInputRate::vertex:
    return VK_VERTEX_INPUT_RATE_VERTEX;

  case VertexInputRate::instance:
    return VK_VERTEX_INPUT_RATE_INSTANCE;
  }

  assert(false);
  return VK_VERTEX_INPUT_RATE_VERTEX;
}

static inline VertexInputRate
from_vk_vertex_input_rate(VkVertexInputRate rate) noexcept {
  switch (rate) {
  case VK_VERTEX_INPUT_RATE_VERTEX:
    return VertexInputRate::vertex;

  case VK_VERTEX_INPUT_RATE_INSTANCE:
    return VertexInputRate::instance;

  default:
    assert(false);
    return VertexInputRate::vertex;
  }
}

} // namespace strobe::gpu
