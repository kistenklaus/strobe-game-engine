#pragma once

#include "strobe/rhi/types/image_aspect.hpp"

#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkImageAspectFlags to_vk_image_aspect(ImageAspect aspects) {
  VkImageAspectFlags result = 0;

  if ((aspects & ImageAspect::color) != ImageAspect::none) {
    result |= VK_IMAGE_ASPECT_COLOR_BIT;
  }
  if ((aspects & ImageAspect::depth) != ImageAspect::none) {
    result |= VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  if ((aspects & ImageAspect::stencil) != ImageAspect::none) {
    result |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  if (result == 0) {
    std::unreachable();
  }

  return result;
}

static inline ImageAspect from_vk_image_aspect(VkImageAspectFlags aspects) {
  constexpr VkImageAspectFlags known_aspects = VK_IMAGE_ASPECT_COLOR_BIT |
                                               VK_IMAGE_ASPECT_DEPTH_BIT |
                                               VK_IMAGE_ASPECT_STENCIL_BIT;

  if (aspects == 0 || (aspects & ~known_aspects) != 0) {
    std::unreachable();
  }

  ImageAspect result = ImageAspect::none;

  if ((aspects & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
    result |= ImageAspect::color;
  }
  if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
    result |= ImageAspect::depth;
  }
  if ((aspects & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
    result |= ImageAspect::stencil;
  }

  return result;
}

} // namespace strobe::rhi
