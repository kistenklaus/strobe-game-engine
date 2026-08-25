#pragma once

#include "strobe/rhi/types/blend_op.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkBlendOp
to_vk_blend_op(BlendOp op) noexcept {
  switch (op) {
  case BlendOp::add:
    return VK_BLEND_OP_ADD;

  case BlendOp::subtract:
    return VK_BLEND_OP_SUBTRACT;

  case BlendOp::reverse_subtract:
    return VK_BLEND_OP_REVERSE_SUBTRACT;

  case BlendOp::min:
    return VK_BLEND_OP_MIN;

  case BlendOp::max:
    return VK_BLEND_OP_MAX;
  }

  assert(false);
  return VK_BLEND_OP_ADD;
}

static inline BlendOp
from_vk_blend_op(VkBlendOp op) noexcept {
  switch (op) {
  case VK_BLEND_OP_ADD:
    return BlendOp::add;
  case VK_BLEND_OP_SUBTRACT:
    return BlendOp::subtract;
  case VK_BLEND_OP_REVERSE_SUBTRACT:
    return BlendOp::reverse_subtract;
  case VK_BLEND_OP_MIN:
    return BlendOp::min;
  case VK_BLEND_OP_MAX:
    return BlendOp::max;
  default:
    assert(false);
    return BlendOp::add;
  }
}

} // namespace strobe::rhi
