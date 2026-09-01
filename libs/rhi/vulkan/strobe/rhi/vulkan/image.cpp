#include "strobe/rhi/vulkan/image.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/memory.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

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
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCreateImage");
#endif
    const VkResult result = vkCreateImage(
        context->device(), &imageInfo, context->driver_alloc(), &image.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create vulkan image");
    }
  }
  return image;
}

void destroy_image(Context *context, Image image) noexcept {
  assert(context);
  assert(image);
#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkDestroyImage");
#endif
  vkDestroyImage(context->device(), image.handle, context->driver_alloc());
}
MemoryRequirements get_image_memory_requirements(Context *context,
                                                 Image image) noexcept {
  assert(context);
  assert(image);
  VkImageMemoryRequirementsInfo2 info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      .pNext = nullptr,
      .image = image.handle,
  };
  VkMemoryDedicatedRequirements dedicatedReq{
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
      .pNext = nullptr,
      .prefersDedicatedAllocation = false,
      .requiresDedicatedAllocation = false,
  };

  VkMemoryRequirements2 req2{
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
      .pNext = &dedicatedReq,
      .memoryRequirements = {},
  };

  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkGetImageMemoryRequirements2");
#endif
    vkGetImageMemoryRequirements2(context->device(), &info, &req2);
  }

  return MemoryRequirements{
      .size = req2.memoryRequirements.size,
      .alignment = req2.memoryRequirements.alignment,
      .memoryTypeBits = req2.memoryRequirements.memoryTypeBits,
      .prefersDedicated =
          static_cast<bool>(dedicatedReq.prefersDedicatedAllocation),
      .requiresDedicated =
          static_cast<bool>(dedicatedReq.requiresDedicatedAllocation),
  };
}

void bind_image_memory(Context *context, const Memory &memory, Image image,
                       VkDeviceSize offset) {
  assert(context != nullptr);
  assert(memory);
  assert(image);
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vmaBindImageMemory2");
#endif
    VkResult result = vmaBindImageMemory2(context->vma(), memory.handle, offset,
                                          image.handle, nullptr);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to bind image memory");
    }
  }
}

} // namespace strobe::rhi::vulkan
