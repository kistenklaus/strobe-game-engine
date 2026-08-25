#pragma once

#include "strobe/rhi/types/image_flags.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageCreateFlags to_vk_image_flags(ImageFlags flags) {
  VkImageCreateFlags result = 0;

  if ((flags & ImageFlags::mutable_format) != 0) {
    result |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
  }

  if ((flags & ImageFlags::cube_compatible) != 0) {
    result |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }

  return result;
}

static inline ImageFlags from_vk_image_flags(VkImageCreateFlags flags) {
  ImageFlags result = ImageFlags::none;

  if ((flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0) {
    result |= ImageFlags::mutable_format;
  }

  if ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0) {
    result |= ImageFlags::cube_compatible;
  }

  return result;
}

} // namespace strobe::rhi
