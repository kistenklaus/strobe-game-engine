#include "strobe/gpu/vulkan/image_view.hpp"

namespace strobe::gpu::vulkan {

ImageView create_image_view(Context *context, const ImageViewInfo &info) {
  assert(context != nullptr);
  assert(info.image);
  assert(info.type != VK_IMAGE_VIEW_TYPE_MAX_ENUM);
  assert(info.format != VK_FORMAT_UNDEFINED);
  assert(info.range.aspectMask != 0);
  assert(info.range.levelCount != 0);
  assert(info.range.layerCount != 0);

  const VkImageViewCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .image = info.image.handle,
      .viewType = info.type,
      .format = info.format,
      .components = info.components,
      .subresourceRange = info.range,
  };
  ImageView view{};
  const VkResult result = vkCreateImageView(
      context->device(), &createInfo, context->driver_alloc(), &view.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create Vulkan image view"};
  }
  return view;
}

void destroy_image_view(Context *context, ImageView view) noexcept {
  assert(context != nullptr);
  assert(view);
  vkDestroyImageView(context->device(), view.handle, context->driver_alloc());
}

} // namespace strobe::gpu::vulkan
