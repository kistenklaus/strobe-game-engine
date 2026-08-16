#pragma once

#include "strobe/gpu/device/logic_op.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkLogicOp
to_vk_logic_op(LogicOp op) noexcept {
  switch (op) {
  case LogicOp::clear:
    return VK_LOGIC_OP_CLEAR;

  case LogicOp::and_:
    return VK_LOGIC_OP_AND;

  case LogicOp::and_reverse:
    return VK_LOGIC_OP_AND_REVERSE;

  case LogicOp::copy:
    return VK_LOGIC_OP_COPY;

  case LogicOp::and_inverted:
    return VK_LOGIC_OP_AND_INVERTED;

  case LogicOp::no_op:
    return VK_LOGIC_OP_NO_OP;

  case LogicOp::xor_:
    return VK_LOGIC_OP_XOR;

  case LogicOp::or_:
    return VK_LOGIC_OP_OR;

  case LogicOp::nor:
    return VK_LOGIC_OP_NOR;

  case LogicOp::equivalent:
    return VK_LOGIC_OP_EQUIVALENT;

  case LogicOp::invert:
    return VK_LOGIC_OP_INVERT;

  case LogicOp::or_reverse:
    return VK_LOGIC_OP_OR_REVERSE;

  case LogicOp::copy_inverted:
    return VK_LOGIC_OP_COPY_INVERTED;

  case LogicOp::or_inverted:
    return VK_LOGIC_OP_OR_INVERTED;

  case LogicOp::nand:
    return VK_LOGIC_OP_NAND;

  case LogicOp::set:
    return VK_LOGIC_OP_SET;
  }

  assert(false);
  return VK_LOGIC_OP_COPY;
}

static inline LogicOp
from_vk_logic_op(VkLogicOp op) noexcept {
  switch (op) {
  case VK_LOGIC_OP_CLEAR:
    return LogicOp::clear;

  case VK_LOGIC_OP_AND:
    return LogicOp::and_;

  case VK_LOGIC_OP_AND_REVERSE:
    return LogicOp::and_reverse;

  case VK_LOGIC_OP_COPY:
    return LogicOp::copy;

  case VK_LOGIC_OP_AND_INVERTED:
    return LogicOp::and_inverted;

  case VK_LOGIC_OP_NO_OP:
    return LogicOp::no_op;

  case VK_LOGIC_OP_XOR:
    return LogicOp::xor_;

  case VK_LOGIC_OP_OR:
    return LogicOp::or_;

  case VK_LOGIC_OP_NOR:
    return LogicOp::nor;

  case VK_LOGIC_OP_EQUIVALENT:
    return LogicOp::equivalent;

  case VK_LOGIC_OP_INVERT:
    return LogicOp::invert;

  case VK_LOGIC_OP_OR_REVERSE:
    return LogicOp::or_reverse;

  case VK_LOGIC_OP_COPY_INVERTED:
    return LogicOp::copy_inverted;

  case VK_LOGIC_OP_OR_INVERTED:
    return LogicOp::or_inverted;

  case VK_LOGIC_OP_NAND:
    return LogicOp::nand;

  case VK_LOGIC_OP_SET:
    return LogicOp::set;

  default:
    assert(false);
    return LogicOp::copy;
  }
}

} // namespace strobe::gpu
