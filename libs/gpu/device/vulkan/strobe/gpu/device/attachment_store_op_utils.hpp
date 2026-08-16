#pragma once

#include "strobe/gpu/device/attachment_store_op.hpp"

#include <cassert>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkAttachmentStoreOp
to_vk_attachment_store_op(AttachmentStoreOp op) noexcept {
  switch (op) {
  case AttachmentStoreOp::store:
    return VK_ATTACHMENT_STORE_OP_STORE;

  case AttachmentStoreOp::dont_care:
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;

  case AttachmentStoreOp::none:
    return VK_ATTACHMENT_STORE_OP_NONE;
  }

  assert(false);
  return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

static inline AttachmentStoreOp
from_vk_attachment_store_op(VkAttachmentStoreOp op) noexcept {
  switch (op) {
  case VK_ATTACHMENT_STORE_OP_STORE:
    return AttachmentStoreOp::store;

  case VK_ATTACHMENT_STORE_OP_DONT_CARE:
    return AttachmentStoreOp::dont_care;

  case VK_ATTACHMENT_STORE_OP_NONE:
    return AttachmentStoreOp::none;

  default:
    assert(false);
    return AttachmentStoreOp::dont_care;
  }
}

} // namespace strobe::gpu
