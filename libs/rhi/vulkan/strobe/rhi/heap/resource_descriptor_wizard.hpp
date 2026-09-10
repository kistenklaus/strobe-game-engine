#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_impl.hpp"
#include "strobe/rhi/img/image_impl.hpp"
#include "strobe/rhi/object_factory.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/image_aspect_utils.hpp"
#include "strobe/rhi/utils/image_layout_utils.hpp"
#include "strobe/rhi/utils/image_view_type_utils.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

class ResourceDescriptorWizard {
public:
  using descriptor = ResourceDescriptor;

  uint64_t size() const noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    return impl->layout.descriptor_stride();
  }
  uint64_t alignment() const noexcept {
    return size(); // may be a tigher but this still definitely be fine.
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    vulkan::Context *ctx = m_heap.ctx();
    Buffer heap_buf = m_heap.buffer();
    auto *heapbuf_impl = object_handle_ptr<BufferImpl>(heap_buf);
    heapbuf_impl->commit();

    Resource resource;

    if (std::holds_alternative<StorageBufferDescriptorInfo>(m_info)) {
      auto &info = std::get<StorageBufferDescriptorInfo>(m_info);
      resource = info.buffer;
      auto *buf_impl = object_handle_ptr<BufferImpl>(info.buffer);
      buf_impl->commit();
      assert(info.offset < buf_impl->size);
      uint64_t buf_size = info.size;
      if (buf_size == std::numeric_limits<uint64_t>::max()) {
        buf_size = buf_impl->size - info.offset;
      }
      VkDeviceAddressRangeKHR address{
          .address = buf_impl->address + info.offset,
          .size = buf_size,
      };
      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .data =
              VkResourceDescriptorDataEXT{
                  .pAddressRange = &address,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }

    } else if (std::holds_alternative<StorageTexelBufferDescriptorInfo>(
                   m_info)) {
      auto &info = std::get<StorageTexelBufferDescriptorInfo>(m_info);
      resource = info.buffer;
      auto *buf_impl = object_handle_ptr<BufferImpl>(info.buffer);
      buf_impl->commit();
      assert(info.offset < buf_impl->size);
      uint64_t buf_size = info.size;
      if (buf_size == std::numeric_limits<uint64_t>::max()) {
        buf_size = buf_impl->size - info.offset;
      }
      VkDeviceAddressRangeKHR address{
          .address = buf_impl->address + info.offset,
          .size = buf_size,
      };
      VkTexelBufferDescriptorInfoEXT texelBuffer{
          .sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .format = to_vk_format(info.format),
          .addressRange = address,
      };
      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
          .data =
              VkResourceDescriptorDataEXT{
                  .pTexelBuffer = &texelBuffer,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }

    } else if (std::holds_alternative<UniformBufferDescriptorInfo>(m_info)) {
      auto &info = std::get<UniformBufferDescriptorInfo>(m_info);
      resource = info.buffer;
      auto *buf_impl = object_handle_ptr<BufferImpl>(info.buffer);
      buf_impl->commit();
      assert(info.offset < buf_impl->size);
      uint64_t buf_size = info.size;
      if (buf_size == std::numeric_limits<uint64_t>::max()) {
        buf_size = buf_impl->size - info.offset;
      }
      VkDeviceAddressRangeKHR address{
          .address = buf_impl->address + info.offset,
          .size = buf_size,
      };
      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .data =
              VkResourceDescriptorDataEXT{
                  .pAddressRange = &address,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }

    } else if (std::holds_alternative<UniformTexelBufferDescriptorInfo>(
                   m_info)) {
      auto &info = std::get<UniformTexelBufferDescriptorInfo>(m_info);
      resource = info.buffer;
      auto *buf_impl = object_handle_ptr<BufferImpl>(info.buffer);
      buf_impl->commit();
      assert(info.offset < buf_impl->size);
      uint64_t buf_size = info.size;
      if (buf_size == std::numeric_limits<uint64_t>::max()) {
        buf_size = buf_impl->size - info.offset;
      }
      VkDeviceAddressRangeKHR address{
          .address = buf_impl->address + info.offset,
          .size = buf_size,
      };
      VkTexelBufferDescriptorInfoEXT texelBuffer{
          .sType = VK_STRUCTURE_TYPE_TEXEL_BUFFER_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .format = to_vk_format(info.format),
          .addressRange = address,
      };
      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
          .data =
              VkResourceDescriptorDataEXT{
                  .pTexelBuffer = &texelBuffer,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }

    } else if (std::holds_alternative<SampledImageDescriptorInfo>(m_info)) {
      auto &info = std::get<SampledImageDescriptorInfo>(m_info);
      resource = info.image;
      auto *img_impl = object_handle_ptr<ImageImpl>(info.image);
      img_impl->commit();
      assert(info.layout != ImageLayout::undefined);
      ImageViewType type = info.viewType;
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
      Format format = info.format;
      if (format == Format::undefined) {
        format = img_impl->format;
      }
      ImageAspect aspect = info.subresource.aspect;
      assert(aspect != ImageAspect::none);
      uint32_t levelCount = info.subresource.levelCount;
      if (levelCount == REMAINING_MIP_LEVELS) {
        levelCount = img_impl->mip_levels - info.subresource.baseMipLevel;
      }
      uint32_t layerCount = info.subresource.layerCount;
      if (layerCount == REMAINING_ARRAY_LAYERS) {
        layerCount = img_impl->arrayLayers - info.subresource.baseArrayLayer;
      }

      VkImageViewCreateInfo view{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = img_impl->image.handle,
          .viewType = to_vk_image_view_type(type),
          .format = to_vk_format(format),
          .components =
              {
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY,
              },
          .subresourceRange =
              {
                  .aspectMask = to_vk_image_aspect(aspect),
                  .baseMipLevel = info.subresource.baseMipLevel,
                  .levelCount = levelCount,
                  .baseArrayLayer = info.subresource.baseArrayLayer,
                  .layerCount = layerCount,
              },

      };

      VkImageDescriptorInfoEXT imageInfo{
          .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .pView = &view,
          .layout = to_vk_image_layout(info.layout),
      };

      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .data =
              VkResourceDescriptorDataEXT{
                  .pImage = &imageInfo,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }

    } else if (std::holds_alternative<StorageImageDescriptorInfo>(m_info)) {
      auto &info = std::get<StorageImageDescriptorInfo>(m_info);
      resource = info.image;
      auto *img_impl = object_handle_ptr<ImageImpl>(info.image);
      img_impl->commit();
      assert(info.layout != ImageLayout::undefined);
      ImageViewType type = info.viewType;
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
      Format format = info.format;
      if (format == Format::undefined) {
        format = img_impl->format;
      }
      ImageAspect aspect = info.subresource.aspect;
      assert(aspect != ImageAspect::none);
      uint32_t levelCount = info.subresource.levelCount;
      if (levelCount == REMAINING_MIP_LEVELS) {
        levelCount = img_impl->mip_levels - info.subresource.baseMipLevel;
      }
      uint32_t layerCount = info.subresource.layerCount;
      if (layerCount == REMAINING_ARRAY_LAYERS) {
        layerCount = img_impl->arrayLayers - info.subresource.baseArrayLayer;
      }

      VkImageViewCreateInfo view{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = img_impl->image.handle,
          .viewType = to_vk_image_view_type(type),
          .format = to_vk_format(format),
          .components =
              {
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY,
              },
          .subresourceRange =
              {
                  .aspectMask = to_vk_image_aspect(aspect),
                  .baseMipLevel = info.subresource.baseMipLevel,
                  .levelCount = levelCount,
                  .baseArrayLayer = info.subresource.baseArrayLayer,
                  .layerCount = layerCount,
              },

      };

      VkImageDescriptorInfoEXT imageInfo{
          .sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .pView = &view,
          .layout = to_vk_image_layout(info.layout),
      };

      VkResourceDescriptorInfoEXT resource{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .data =
              VkResourceDescriptorDataEXT{
                  .pImage = &imageInfo,
              },
      };
      VkHostAddressRangeEXT descriptor{
          .address = dst,
          .size = stride,
      };
      {
#ifdef STROBE_RHI_TRACE_VK
        ZoneScopedN("vkWriteResourceDescriptors");
#endif
        const VkResult result = vulkan::vk_write_resource_descriptors(
            ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to write buffer resource descriptor.");
        }
      }
    }

    Timepoint ready = fn(BufferRange{
        .buffer = heap_buf,
        .offset = offset,
        .size = stride,
    });
    return detail::make_object<ResourceDescriptor>(
        make_void_handle<ResourceDescriptorImpl>(
            &impl->bufferDescAlloc, std::move(m_heap),
            std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
            std::move(ready), std::move(resource)));
  }
  ~ResourceDescriptorWizard() noexcept;

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

private:
  friend class ResourceDescriptorHeap;
  friend class ResourceDescriptorArrayWizard;
  explicit ResourceDescriptorWizard(ResourceDescriptorHeap heap, uint32_t index,
                                    const ResourceDescriptorInfo &info) noexcept
      : m_heap(std::move(heap)), m_index(index), m_info(info) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  const ResourceDescriptorInfo &m_info;
};

} // namespace strobe::rhi
