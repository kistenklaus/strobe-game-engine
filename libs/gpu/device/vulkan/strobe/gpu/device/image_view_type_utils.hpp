#pragma once

#include "strobe/gpu/device/image_view_type.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>
namespace strobe::gpu {

static inline VkImageViewType to_vk_image_view_type(ImageViewType type) {
  switch (type) {
  case ImageViewType::image_1d:
    return VK_IMAGE_VIEW_TYPE_1D;

  case ImageViewType::image_2d:
    return VK_IMAGE_VIEW_TYPE_2D;

  case ImageViewType::image_3d:
    return VK_IMAGE_VIEW_TYPE_3D;

  case ImageViewType::image_1d_array:
    return VK_IMAGE_VIEW_TYPE_1D_ARRAY;

  case ImageViewType::image_2d_array:
    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

  case ImageViewType::cube:
    return VK_IMAGE_VIEW_TYPE_CUBE;

  case ImageViewType::cube_array:
    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

  case ImageViewType::automatic:
    break;
  }

  std::unreachable();
}

static inline ImageViewType from_vk_image_view_type(VkImageViewType type) {
  switch (type) {
  case VK_IMAGE_VIEW_TYPE_1D:
    return ImageViewType::image_1d;

  case VK_IMAGE_VIEW_TYPE_2D:
    return ImageViewType::image_2d;

  case VK_IMAGE_VIEW_TYPE_3D:
    return ImageViewType::image_3d;

  case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
    return ImageViewType::image_1d_array;

  case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
    return ImageViewType::image_2d_array;

  case VK_IMAGE_VIEW_TYPE_CUBE:
    return ImageViewType::cube;

  case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
    return ImageViewType::cube_array;

  default:
    std::unreachable();
  }
}

} // namespace strobe::gpu
