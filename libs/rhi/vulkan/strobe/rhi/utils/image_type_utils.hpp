#pragma once

#include "strobe/rhi/types/image_type.hpp"
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageType to_vk_image_type(ImageType type) {
  switch (type) {
  case ImageType::image_1d:
    return VK_IMAGE_TYPE_1D;
  case ImageType::image_2d:
    return VK_IMAGE_TYPE_2D;
  case ImageType::image_3d:
    return VK_IMAGE_TYPE_3D;
    break;
  }
  std::unreachable();
}

static inline ImageType from_vk_image_type(VkImageType type) {
  switch (type) {
  case VK_IMAGE_TYPE_1D:
    return ImageType::image_1d;
  case VK_IMAGE_TYPE_2D:
    return ImageType::image_2d;
  case VK_IMAGE_TYPE_3D:
    return ImageType::image_3d;
    break;
  case VK_IMAGE_TYPE_MAX_ENUM:
    throw std::invalid_argument("invalid VkImageType");
  }
  std::unreachable();
}

} // namespace strobe::rhi
