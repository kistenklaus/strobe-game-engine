#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation_flags.hpp"
#include "strobe/rhi/memory/memory_granularity_class.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/image_aspect_utils.hpp"
#include "strobe/rhi/utils/image_flags_utils.hpp"
#include "strobe/rhi/utils/image_type_utils.hpp"
#include "strobe/rhi/utils/image_usage_utils.hpp"
#include "strobe/rhi/utils/image_view_type_utils.hpp"
#include "strobe/rhi/utils/sample_count_utils.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include "strobe/rhi/vulkan/image_view.hpp"

namespace strobe::rhi::img {

Image create_image(const MemoryPool &memoryPool, const ImageInfo &info,
                   const MemoryLifetime &lifetime,
                   handle_allocators* alloc) {
  Context context = memoryPool.context();
  vulkan::Context *ctx = context.ctx();

  vulkan::Image image = vulkan::create_image(
      ctx, {
               .type = to_vk_image_type(info.type),
               .format = to_vk_format(info.format),
               .extent =
                   VkExtent3D{
                       .width = info.extent.x(),
                       .height = info.extent.y(),
                       .depth = info.extent.z(),
                   },
               .mip_levels = info.mip_levels,
               .array_layers = info.arrayLayers,
               .samples = to_vk_sample_count(info.samples),
               .tiling = info.linearTiling ? VK_IMAGE_TILING_LINEAR
                                           : VK_IMAGE_TILING_OPTIMAL,
               .usage = to_vk_image_usage(info.imageUsage),
               .flags = to_vk_image_flags(info.flags),
               .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
           });

  vulkan::MemoryRequirements requirements =
      vulkan::get_image_memory_requirements(ctx, image);

  MemoryAllocationFlags flags{};
  if (requirements.prefersDedicated) {
    flags |= MemoryAllocationFlags::prefer_dedicated;
  }
  if (requirements.requiresDedicated) {
    flags |= MemoryAllocationFlags::require_dedicated;
  }

  MemoryAllocation allocation = memoryPool.allocate_memory(
      {
          .size = requirements.size,
          .alignment = requirements.alignment,
          .memoryTypeBits = requirements.memoryTypeBits,
          .granularityClass = info.linearTiling
                                  ? MemoryGranularityClass::linear
                                  : MemoryGranularityClass::optimal,
          .flags = flags,
          .memoryUsage = info.memoryUsage,
          .dedicated = image,
      },
      lifetime);

  if (allocation.binding()) {
    vulkan::bind_image_memory(ctx, allocation.binding().memory, image,
                              allocation.binding().offset);
  }
  return Image{make_void_handle<ImageImpl>(
      &alloc->imageAllocator, std::move(context), std::move(allocation), image, info.type,
      info.format, info.extent, info.mip_levels, info.arrayLayers,
      info.samples)};
}

ImageView create_image_view(Image image, const ImageViewInfo &info,
                            handle_allocators *alloc) {
  auto *img_impl = object_handle_ptr<ImageImpl, Image>(image);
  Context context = img_impl->context;
  vulkan::Context *const ctx = context.ctx();

  const Format format =
      info.format == Format::undefined ? img_impl->format : info.format;

  ImageViewType type = info.type;
  if (type == ImageViewType::none) {
    switch (img_impl->type) {
    case ImageType::image_1d:
      type = ImageViewType::image_1d;
      break;
    case ImageType::image_2d:
      type = ImageViewType::image_2d;
      break;
    case ImageType::image_3d:
      type = ImageViewType::image_3d;
      break;
    }
  }

  const uint32_t mipLevelCount =
      info.range.levelCount == REMAINING_MIP_LEVELS
          ? img_impl->mip_levels - info.range.baseMipLevel
          : info.range.levelCount;

  const uint32_t arrayLayerCount =
      info.range.layerCount == REMAINING_ARRAY_LAYERS
          ? img_impl->arrayLayers - info.range.baseArrayLayer
          : info.range.layerCount;

  vulkan::ImageView view = vulkan::create_image_view(
      ctx, {
               .image = img_impl->image,
               .type = to_vk_image_view_type(type),
               .flags = 0,
               .format = to_vk_format(format),
               .components =
                   {
                       .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                   },
               .range =
                   {
                       .aspectMask = to_vk_image_aspect(info.range.aspect),
                       .baseMipLevel = info.range.baseMipLevel,
                       .levelCount = mipLevelCount,
                       .baseArrayLayer = info.range.baseArrayLayer,
                       .layerCount = arrayLayerCount,
                   },
           });

  return ImageView{make_void_handle<ImageViewImpl>(
      &alloc->imageViewAllocator, std::move(image), view, format)};
}

} // namespace strobe::rhi::img
