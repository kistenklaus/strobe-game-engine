#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_view_impl.hpp"
#include "strobe/rhi/objects/image.hpp"

namespace strobe::rhi {

void ImageView::pin(void *handle) noexcept {
  pin_void_handle<ImageViewImpl>(handle);
}
void ImageView::unpin(void *handle) noexcept {
  unpin_void_handle<ImageViewImpl>(handle);
}

Format ImageView::format() const noexcept {
  auto *impl = void_handle_ptr<ImageViewImpl>(m_handle);
  return impl->format;
}

const Image &ImageView::image() const noexcept {
  auto *impl = void_handle_ptr<ImageViewImpl>(m_handle);
  return impl->image;
}

} // namespace strobe::rhi
