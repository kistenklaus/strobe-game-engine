#pragma once

#include "strobe/rhi/types/image_aspect.hpp"
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageAspectFlags to_vk_image_aspect(ImageAspect aspect) {
  switch (aspect) {
  case ImageAspect::color:
    return VK_IMAGE_ASPECT_COLOR_BIT;

  case ImageAspect::depth:
    return VK_IMAGE_ASPECT_DEPTH_BIT;

  case ImageAspect::stencil:
    return VK_IMAGE_ASPECT_STENCIL_BIT;

  case ImageAspect::depth_stencil:
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  std::unreachable();
}

static inline ImageAspect from_vk_image_aspect(VkImageAspectFlags aspect) {
  switch (aspect) {
  case VK_IMAGE_ASPECT_COLOR_BIT:
    return ImageAspect::color;

  case VK_IMAGE_ASPECT_DEPTH_BIT:
    return ImageAspect::depth;

  case VK_IMAGE_ASPECT_STENCIL_BIT:
    return ImageAspect::stencil;

  case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
    return ImageAspect::depth_stencil;

  default:
    std::unreachable();
  }
}

} // namespace strobe::rhi
