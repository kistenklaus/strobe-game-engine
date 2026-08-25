#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/vulkan/image.hpp"

namespace strobe::rhi {

struct ImageImpl {

  ImageImpl(Context context, MemoryAllocation allocation, vulkan::Image image,
            ImageType type, Format format, uvec3 extent, uint32_t mip_levels,
            uint32_t arrayLayers, SampleCount samples)
      : context(std::move(context)), allocation(std::move(allocation)),
        image(image), type(type), format(format), extent(extent),
        mip_levels(mip_levels), arrayLayers(arrayLayers), samples(samples) {}
  ImageImpl(const ImageImpl &) = delete;
  ImageImpl(ImageImpl &&) = delete;
  ImageImpl &operator=(const ImageImpl &) = delete;
  ImageImpl &operator=(ImageImpl &&) = delete;

  ~ImageImpl() noexcept {
    if (allocation) {
      vulkan::destroy_image(context.ctx(), image);
    }
  }

  const Context context;
  const MemoryAllocation allocation;
  const vulkan::Image image; // already contains the vma allocation
  const ImageType type;
  const Format format;
  const uvec3 extent;
  const uint32_t mip_levels;
  const uint32_t arrayLayers;
  const SampleCount samples;
};

} // namespace strobe::rhi
