#pragma once

#include "strobe/rhi/vulkan/image.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct ImageView {
  VkImageView handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct ImageViewInfo {
  Image image{};
  VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D;
  VkImageViewCreateFlags flags = 0;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkComponentMapping components{
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
  };
  VkImageSubresourceRange range{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };
};

ImageView create_image_view(Context *context, const ImageViewInfo &info);

void destroy_image_view(Context *context, ImageView view) noexcept;

} // namespace strobe::rhi::vulkan
