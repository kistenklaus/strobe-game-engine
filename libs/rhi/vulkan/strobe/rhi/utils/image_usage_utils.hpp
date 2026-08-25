#pragma once

#include "strobe/rhi/types/image_usage.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageUsageFlags to_vk_image_usage(ImageUsage usage) {
  VkImageUsageFlags result = 0;

  if ((usage & ImageUsage::transfer_src) != 0)
    result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  if ((usage & ImageUsage::transfer_dst) != 0)
    result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  if ((usage & ImageUsage::sampled) != 0)
    result |= VK_IMAGE_USAGE_SAMPLED_BIT;

  if ((usage & ImageUsage::storage) != 0)
    result |= VK_IMAGE_USAGE_STORAGE_BIT;

  if ((usage & ImageUsage::color_attachment) != 0)
    result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if ((usage & ImageUsage::depth_stencil) != 0)
    result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  return result;
}

}
