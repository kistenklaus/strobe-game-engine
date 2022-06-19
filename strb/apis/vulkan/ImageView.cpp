#include "strb/apis/vulkan/ImageView.hpp"

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

ImageView::ImageView(const Device& device, const Image image,
                     const Format colorFormat)
    : device(&device) {
  VkImageViewCreateInfo imageViewCreateInfo;
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.pNext = nullptr;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format::toVkFormat(colorFormat);
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  VkResult result = vkCreateImageView(*(this->device), &imageViewCreateInfo,
                                      nullptr, &this->imageView);
  ASSERT_VKRESULT(result);
}

void ImageView::destroy() {
  assert(this->device != nullptr);
  assert(this->imageView != VK_NULL_HANDLE);
  vkDestroyImageView(*(this->device), this->imageView, nullptr);
  dexec(this->device = nullptr);
  dexec(this->imageView = VK_NULL_HANDLE);
}

}  // namespace strb::vulkan
