#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_impl.hpp"
#include "strobe/rhi/img/image_view_impl.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/types/image_info.hpp"
#include "strobe/rhi/types/image_view_info.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::img {

struct handle_allocators {
  handle_allocator<ImageImpl> imageAllocator;
  handle_allocator<ImageView> imageViewAllocator;
};

Image create_image(const MemoryPool &memoryPool, const ImageInfo &info,
                   const MemoryLifetime &lifetime,
                   handle_allocator_ref<ImageImpl> alloc);

ImageView create_image_view(Image image, const ImageViewInfo &info,
                            handle_allocator_ref<ImageViewImpl> alloc);

} // namespace strobe::rhi::img
