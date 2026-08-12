#include "strobe/gpu/device/image_view.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/image_view_impl.hpp"

namespace strobe::gpu {

ImageView::ImageView(const ImageView &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ImageViewImpl>(m_handle);
  }
}

ImageView::ImageView(ImageView &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

ImageView &ImageView::operator=(const ImageView &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ImageViewImpl>(o.m_handle);
  }
  unpin_void_handle<ImageViewImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ImageView &ImageView::operator=(ImageView &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ImageViewImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ImageView::~ImageView() noexcept { unpin_void_handle<ImageViewImpl>(m_handle); }

Format ImageView::format() const noexcept {
  auto *impl = void_handle_ptr<ImageViewImpl>(m_handle);
  return impl->format;
}

Image ImageView::image() const noexcept {
  auto *impl = void_handle_ptr<ImageViewImpl>(m_handle);
  return impl->image;
}

} // namespace strobe::gpu
