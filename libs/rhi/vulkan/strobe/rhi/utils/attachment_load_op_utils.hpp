#pragma once

#include "strobe/rhi/types/attachment_load_op.hpp"
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkAttachmentLoadOp
to_vk_attachment_load_op(AttachmentLoadOp op) noexcept {
  switch (op) {
  case AttachmentLoadOp::load:
    return VK_ATTACHMENT_LOAD_OP_LOAD;

  case AttachmentLoadOp::clear:
    return VK_ATTACHMENT_LOAD_OP_CLEAR;

  case AttachmentLoadOp::dont_care:
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  case AttachmentLoadOp::none:
    return VK_ATTACHMENT_LOAD_OP_NONE;
  }

  assert(false);
  return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

static inline AttachmentLoadOp
from_vk_attachment_load_op(VkAttachmentLoadOp op) noexcept {
  switch (op) {
  case VK_ATTACHMENT_LOAD_OP_LOAD:
    return AttachmentLoadOp::load;

  case VK_ATTACHMENT_LOAD_OP_CLEAR:
    return AttachmentLoadOp::clear;

  case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
    return AttachmentLoadOp::dont_care;

  case VK_ATTACHMENT_LOAD_OP_NONE:
    return AttachmentLoadOp::none;

  default:
    assert(false);
    return AttachmentLoadOp::dont_care;
  }
}

} // namespace strobe::rhi
