#include "strobe/gpu/vulkan/image.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"

namespace strobe::gpu::vulkan {

Image create_image(Context *context, const ImageInfo &info) {
  assert(context != nullptr);
  assert(info.format != VK_FORMAT_UNDEFINED);
  assert(info.extent.width != 0);
  assert(info.extent.height != 0);
  assert(info.extent.depth != 0);
  assert(info.mip_levels != 0);
  assert(info.array_layers != 0);
  assert(info.usage != 0);

  const VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .imageType = info.type,
      .format = info.format,
      .extent = info.extent,
      .mipLevels = info.mip_levels,
      .arrayLayers = info.array_layers,
      .samples = info.samples,
      .tiling = info.tiling,
      .usage = info.usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = info.initial_layout,
  };
  Image image{};
  const VkResult result =
      vmaCreateImage(context->vma(), &imageInfo,
                     details::get_allocation_create_info(info.memory_usage),
                     &image.handle, &image.allocation, nullptr);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create Vulkan image"};
  }
  return image;
}

void destroy_image(Context *context, Image image) noexcept {
  assert(context != nullptr);
  assert(image);
  if (image.allocation != VK_NULL_HANDLE) { // externally owned!
    vmaDestroyImage(context->vma(), image.handle, image.allocation);
  }
}

void *map_image(Context *context, Image image) {
  assert(context != nullptr);
  assert(image);
  assert(image.allocation != VK_NULL_HANDLE);
  void *data = nullptr;
  const VkResult result = vmaMapMemory(context->vma(), image.allocation, &data);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to map Vulkan image"};
  }
  return data;
}

void unmap_image(Context *context, Image image) noexcept {
  assert(context != nullptr);
  assert(image);
  assert(image.allocation != VK_NULL_HANDLE);
  vmaUnmapMemory(context->vma(), image.allocation);
}

void flush_image(Context *context, Image image, VkDeviceSize offset,
                 VkDeviceSize size) {
  assert(context != nullptr);
  assert(image);
  assert(image.allocation != VK_NULL_HANDLE);
  const VkResult result =
      vmaFlushAllocation(context->vma(), image.allocation, offset, size);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to flush Vulkan image allocation"};
  }
}

void invalidate_image(Context *context, Image image, VkDeviceSize offset,
                      VkDeviceSize size) {
  assert(context != nullptr);
  assert(image);
  assert(image.allocation != VK_NULL_HANDLE);
  const VkResult result =
      vmaInvalidateAllocation(context->vma(), image.allocation, offset, size);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to invalidate Vulkan image allocation"};
  }
}

void *get_persistently_mapped_image_ptr(Context *context,
                                        Image image) noexcept {
  assert(context != nullptr);
  assert(image);
  assert(image.allocation != VK_NULL_HANDLE);
  VmaAllocationInfo allocationInfo{};
  vmaGetAllocationInfo(context->vma(), image.allocation, &allocationInfo);
  assert(allocationInfo.pMappedData != nullptr);
  return allocationInfo.pMappedData;
}

[[nodiscard]]
VkSubresourceLayout
get_image_subresource_layout(Context *context, Image image,
                             const VkImageSubresource &subresource) noexcept {
  assert(context != nullptr);
  assert(image);
  VkSubresourceLayout layout{};
  vkGetImageSubresourceLayout(context->device(), image.handle, &subresource,
                              &layout);
  return layout;
}

} // namespace strobe::gpu::vulkan
