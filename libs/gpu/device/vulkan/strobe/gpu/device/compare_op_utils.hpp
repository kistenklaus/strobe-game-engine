#pragma once

#include "strobe/gpu/device/compare_op.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkCompareOp
to_vk_compare_op(CompareOp op) noexcept {
  switch (op) {
  case CompareOp::never:
    return VK_COMPARE_OP_NEVER;

  case CompareOp::less:
    return VK_COMPARE_OP_LESS;

  case CompareOp::equal:
    return VK_COMPARE_OP_EQUAL;

  case CompareOp::less_or_equal:
    return VK_COMPARE_OP_LESS_OR_EQUAL;

  case CompareOp::greater:
    return VK_COMPARE_OP_GREATER;

  case CompareOp::not_equal:
    return VK_COMPARE_OP_NOT_EQUAL;

  case CompareOp::greater_or_equal:
    return VK_COMPARE_OP_GREATER_OR_EQUAL;

  case CompareOp::always:
    return VK_COMPARE_OP_ALWAYS;
  }

  assert(false);
  return VK_COMPARE_OP_ALWAYS;
}

static inline CompareOp
from_vk_compare_op(VkCompareOp op) noexcept {
  switch (op) {
  case VK_COMPARE_OP_NEVER:
    return CompareOp::never;

  case VK_COMPARE_OP_LESS:
    return CompareOp::less;

  case VK_COMPARE_OP_EQUAL:
    return CompareOp::equal;

  case VK_COMPARE_OP_LESS_OR_EQUAL:
    return CompareOp::less_or_equal;

  case VK_COMPARE_OP_GREATER:
    return CompareOp::greater;

  case VK_COMPARE_OP_NOT_EQUAL:
    return CompareOp::not_equal;

  case VK_COMPARE_OP_GREATER_OR_EQUAL:
    return CompareOp::greater_or_equal;

  case VK_COMPARE_OP_ALWAYS:
    return CompareOp::always;

  default:
    assert(false);
    return CompareOp::always;
  }
}

} // namespace strobe::gpu
