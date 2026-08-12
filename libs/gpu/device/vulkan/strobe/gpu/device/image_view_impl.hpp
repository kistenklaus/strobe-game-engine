#pragma once

#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_impl.hpp"
#include "strobe/gpu/vulkan/image_view.hpp"
namespace strobe::gpu {

struct ImageViewImpl {

  ImageViewImpl(Image image, vulkan::ImageView view, Format format)
      : image(std::move(image)), imageView(view), format(format) {}
  ~ImageViewImpl() {
    auto *image_impl = void_handle_ptr<ImageImpl>(image.m_handle);
    auto *device_impl =
        void_handle_ptr<DeviceImpl>(image_impl->device.m_handle);

    vulkan::destroy_image_view(&device_impl->context, imageView);
  }

  const Image image;
  const vulkan::ImageView imageView;
  const Format format;
};

} // namespace strobe::gpu
