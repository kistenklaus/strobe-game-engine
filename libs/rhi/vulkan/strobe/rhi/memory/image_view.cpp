#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/image_view_impl.hpp"
#include "strobe/rhi/objects/image.hpp"

namespace strobe::rhi {

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

const Image &ImageView::image() const noexcept {
  auto *impl = void_handle_ptr<ImageViewImpl>(m_handle);
  return impl->image;
}

} // namespace strobe::rhi
