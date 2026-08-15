#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/format.hpp"
#include "strobe/gpu/vulkan/image.hpp"
namespace strobe::gpu {

struct ImageImpl {

  ImageImpl(Context context, vulkan::Image image, ImageType type, Format format,
            uvec3 extent, uint32_t mip_levels, uint32_t arrayLayers,
            SampleCount samples)
      : context(std::move(context)), image(image), type(type), format(format),
        extent(extent), mip_levels(mip_levels), arrayLayers(arrayLayers),
        samples(samples) {}
  ImageImpl(const ImageImpl &) = delete;
  ImageImpl(ImageImpl &&) = delete;
  ImageImpl &operator=(const ImageImpl &) = delete;
  ImageImpl &operator=(ImageImpl &&) = delete;

  ~ImageImpl() { vulkan::destroy_image(context.get(), image); }

  const Context context;
  const vulkan::Image image; // already contains the vma allocation
  const ImageType type;
  const Format format;
  const uvec3 extent;
  const uint32_t mip_levels;
  const uint32_t arrayLayers;
  const SampleCount samples;
};

} // namespace strobe::gpu
