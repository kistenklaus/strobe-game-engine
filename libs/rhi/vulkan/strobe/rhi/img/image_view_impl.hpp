#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_impl.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/vulkan/image_view.hpp"

namespace strobe::rhi {

struct ImageViewImpl {
  ImageViewImpl(Image image, vulkan::ImageView view, Format format)
      : image(std::move(image)), imageView(view), format(format) {}
  ~ImageViewImpl() {
    auto *image_impl = void_handle_ptr<ImageImpl>(image.m_handle);
    vulkan::destroy_image_view(image_impl->context.ctx(), imageView);
  }

  const Image image;
  const vulkan::ImageView imageView;
  const Format format;
};

} // namespace strobe::rhi
