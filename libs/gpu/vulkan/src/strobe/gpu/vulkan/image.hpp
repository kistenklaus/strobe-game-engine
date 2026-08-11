#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Image {
  VkImage handle = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;

  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct ImageInfo {
  VkImageType type = VK_IMAGE_TYPE_2D;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkExtent3D extent{
      .width = 1,
      .height = 1,
      .depth = 1,
  };
  uint32_t mip_levels = 1;
  uint32_t array_layers = 1;
  VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags usage = 0;
  VkImageCreateFlags flags = 0;
  VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  MemoryUsage memory_usage = MemoryUsage::automatic;
};

Image create_image(Context *context, const ImageInfo &info);
void destroy_image(Context *context, Image image) noexcept;
void *map_image(Context *context, Image image);
void unmap_image(Context *context, Image image) noexcept;
void flush_image(Context *context, Image image, VkDeviceSize offset = 0,
                 VkDeviceSize size = VK_WHOLE_SIZE);
void invalidate_image(Context *context, Image image, VkDeviceSize offset = 0,
                      VkDeviceSize size = VK_WHOLE_SIZE);
void *get_persistently_mapped_image_ptr(Context *context, Image image) noexcept;

VkSubresourceLayout get_image_subresource_layout(
    Context *context, Image image,
    const VkImageSubresource &subresource = VkImageSubresource{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}) noexcept;

} // namespace strobe::gpu::vulkan
