#pragma once

#include "strobe/gpu/device/blend_factor.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkBlendFactor
to_vk_blend_factor(BlendFactor factor) noexcept {
  switch (factor) {
  case BlendFactor::zero:
    return VK_BLEND_FACTOR_ZERO;

  case BlendFactor::one:
    return VK_BLEND_FACTOR_ONE;

  case BlendFactor::src_color:
    return VK_BLEND_FACTOR_SRC_COLOR;

  case BlendFactor::one_minus_src_color:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

  case BlendFactor::dst_color:
    return VK_BLEND_FACTOR_DST_COLOR;

  case BlendFactor::one_minus_dst_color:
    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

  case BlendFactor::src_alpha:
    return VK_BLEND_FACTOR_SRC_ALPHA;

  case BlendFactor::one_minus_src_alpha:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

  case BlendFactor::dst_alpha:
    return VK_BLEND_FACTOR_DST_ALPHA;

  case BlendFactor::one_minus_dst_alpha:
    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;

  case BlendFactor::constant_color:
    return VK_BLEND_FACTOR_CONSTANT_COLOR;

  case BlendFactor::one_minus_constant_color:
    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

  case BlendFactor::constant_alpha:
    return VK_BLEND_FACTOR_CONSTANT_ALPHA;

  case BlendFactor::one_minus_constant_alpha:
    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;

  case BlendFactor::src_alpha_saturate:
    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;

  case BlendFactor::src1_color:
    return VK_BLEND_FACTOR_SRC1_COLOR;

  case BlendFactor::one_minus_src1_color:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;

  case BlendFactor::src1_alpha:
    return VK_BLEND_FACTOR_SRC1_ALPHA;

  case BlendFactor::one_minus_src1_alpha:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
  }

  assert(false);
  return VK_BLEND_FACTOR_ONE;
}

static inline BlendFactor
from_vk_blend_factor(VkBlendFactor factor) noexcept {
  switch (factor) {
  case VK_BLEND_FACTOR_ZERO:
    return BlendFactor::zero;

  case VK_BLEND_FACTOR_ONE:
    return BlendFactor::one;

  case VK_BLEND_FACTOR_SRC_COLOR:
    return BlendFactor::src_color;

  case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
    return BlendFactor::one_minus_src_color;

  case VK_BLEND_FACTOR_DST_COLOR:
    return BlendFactor::dst_color;

  case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
    return BlendFactor::one_minus_dst_color;

  case VK_BLEND_FACTOR_SRC_ALPHA:
    return BlendFactor::src_alpha;

  case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
    return BlendFactor::one_minus_src_alpha;

  case VK_BLEND_FACTOR_DST_ALPHA:
    return BlendFactor::dst_alpha;

  case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
    return BlendFactor::one_minus_dst_alpha;

  case VK_BLEND_FACTOR_CONSTANT_COLOR:
    return BlendFactor::constant_color;

  case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
    return BlendFactor::one_minus_constant_color;

  case VK_BLEND_FACTOR_CONSTANT_ALPHA:
    return BlendFactor::constant_alpha;

  case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
    return BlendFactor::one_minus_constant_alpha;

  case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
    return BlendFactor::src_alpha_saturate;

  case VK_BLEND_FACTOR_SRC1_COLOR:
    return BlendFactor::src1_color;

  case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
    return BlendFactor::one_minus_src1_color;

  case VK_BLEND_FACTOR_SRC1_ALPHA:
    return BlendFactor::src1_alpha;

  case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
    return BlendFactor::one_minus_src1_alpha;

  default:
    assert(false);
    return BlendFactor::one;
  }
}

} // namespace strobe::gpu
