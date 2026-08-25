#pragma once

#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/memory_requirements.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {
struct Memory;

struct Image {
  VkImage handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
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
};

Image create_image(Context *context, const ImageInfo &info);

void destroy_image(Context *context, Image image) noexcept;

MemoryRequirements get_image_memory_requirements(Context *context,
                                                 Image image) noexcept;

void bind_image_memory(Context *context, const Memory &memory, Image image,
                       VkDeviceSize offset);

} // namespace strobe::rhi::vulkan
