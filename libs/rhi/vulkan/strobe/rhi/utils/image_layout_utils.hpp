#pragma once

#include "strobe/rhi/types/image_layout.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageLayout to_vk_image_layout(ImageLayout layout) {
  switch (layout) {
  case ImageLayout::undefined:
    return VK_IMAGE_LAYOUT_UNDEFINED;

  case ImageLayout::general:
    return VK_IMAGE_LAYOUT_GENERAL;

  case ImageLayout::read_only:
    return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

  case ImageLayout::attachment:
    return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

  case ImageLayout::transfer_src:
    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  case ImageLayout::transfer_dst:
    return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

  case ImageLayout::present:
    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }

  return VK_IMAGE_LAYOUT_UNDEFINED;
}

static inline ImageLayout from_vk_image_layout(VkImageLayout layout) {
  switch (layout) {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    return ImageLayout::undefined;

  case VK_IMAGE_LAYOUT_GENERAL:
    return ImageLayout::general;

  case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
    return ImageLayout::read_only;

  case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
    return ImageLayout::attachment;

  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    return ImageLayout::transfer_src;

  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    return ImageLayout::transfer_dst;

  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    return ImageLayout::present;

  default:
    return ImageLayout::undefined;
  }
}

} // namespace strobe::rhi
