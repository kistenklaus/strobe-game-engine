#pragma once

#include "strobe/gpu/device/stencil_op.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkStencilOp to_vk_stencil_op(StencilOp op) noexcept {
  switch (op) {
  case StencilOp::keep:
    return VK_STENCIL_OP_KEEP;

  case StencilOp::zero:
    return VK_STENCIL_OP_ZERO;

  case StencilOp::replace:
    return VK_STENCIL_OP_REPLACE;

  case StencilOp::increment_and_clamp:
    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;

  case StencilOp::decrement_and_clamp:
    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;

  case StencilOp::invert:
    return VK_STENCIL_OP_INVERT;

  case StencilOp::increment_and_wrap:
    return VK_STENCIL_OP_INCREMENT_AND_WRAP;

  case StencilOp::decrement_and_wrap:
    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
  }

  assert(false);
  return VK_STENCIL_OP_KEEP;
}

static inline StencilOp from_vk_stencil_op(VkStencilOp op) noexcept {
  switch (op) {
  case VK_STENCIL_OP_KEEP:
    return StencilOp::keep;

  case VK_STENCIL_OP_ZERO:
    return StencilOp::zero;

  case VK_STENCIL_OP_REPLACE:
    return StencilOp::replace;

  case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
    return StencilOp::increment_and_clamp;

  case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
    return StencilOp::decrement_and_clamp;

  case VK_STENCIL_OP_INVERT:
    return StencilOp::invert;

  case VK_STENCIL_OP_INCREMENT_AND_WRAP:
    return StencilOp::increment_and_wrap;

  case VK_STENCIL_OP_DECREMENT_AND_WRAP:
    return StencilOp::decrement_and_wrap;

  default:
    assert(false);
    return StencilOp::keep;
  }
}

} // namespace strobe::gpu
