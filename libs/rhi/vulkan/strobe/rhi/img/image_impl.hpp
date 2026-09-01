#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_type.hpp"
#include "strobe/rhi/types/sample_count.hpp"
#include "strobe/rhi/vulkan/image.hpp"

namespace strobe::rhi {

struct ImageImpl {

  ImageImpl(Context context, MemoryAllocation allocation, vulkan::Image image,
            ImageType type, Format format, uvec3 extent, uint32_t mip_levels,
            uint32_t arrayLayers, SampleCount samples)
      : context(std::move(context)), allocation(std::move(allocation)),
        image(image), type(type), format(format), extent(extent),
        mip_levels(mip_levels), arrayLayers(arrayLayers), samples(samples) {
  }
  ImageImpl(const ImageImpl &) = delete;
  ImageImpl(ImageImpl &&) = delete;
  ImageImpl &operator=(const ImageImpl &) = delete;
  ImageImpl &operator=(ImageImpl &&) = delete;

  ~ImageImpl() noexcept {
    if (allocation) {
      vulkan::destroy_image(context.ctx(), image);
    }
  }

  bool commit() {
    if (allocation.commit()) {
      vulkan::bind_image_memory(context.ctx(), allocation.binding().memory,
                                image, allocation.binding().offset);
      return true;
    }
    return false;
  }

  const Context context;
  const MemoryAllocation allocation;
  const vulkan::Image image;
  const ImageType type;
  const Format format;
  const uvec3 extent;
  const uint32_t mip_levels;
  const uint32_t arrayLayers;
  const SampleCount samples;
};

} // namespace strobe::rhi
