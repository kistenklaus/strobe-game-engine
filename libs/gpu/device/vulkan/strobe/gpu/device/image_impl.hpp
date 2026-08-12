#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/format.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/image.hpp"
namespace strobe::gpu {

struct ImageImpl {

  ImageImpl(Device device, vulkan::Image image, ImageType type, Format format,
            uvec3 extent, uint32_t mip_levels, uint32_t arrayLayers,
            SampleCount samples)
      : device(std::move(device)), image(image), type(type), format(format), extent(extent),
        mip_levels(mip_levels), arrayLayers(arrayLayers), samples(samples)
        {}
  ImageImpl(const ImageImpl &) = delete;
  ImageImpl(ImageImpl &&) = delete;
  ImageImpl &operator=(const ImageImpl &) = delete;
  ImageImpl &operator=(ImageImpl &&) = delete;

  ~ImageImpl() {
    auto *device_impl = void_handle_ptr<DeviceImpl>(device.m_handle);
    // does nothing if image.allocation is VK_NULL_HANDLE!
    vulkan::destroy_image(&device_impl->context, image);
  }

  const Device device;
  const vulkan::Image image; // already contains the vma allocation
  const ImageType type;
  const Format format;
  const uvec3 extent;
  const uint32_t mip_levels;
  const uint32_t arrayLayers;
  const SampleCount samples;

};

} // namespace strobe::gpu
