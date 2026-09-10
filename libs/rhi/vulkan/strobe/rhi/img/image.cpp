#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/img/image_impl.hpp"

namespace strobe::rhi {

void Image::pin(void *handle) noexcept { pin_void_handle<ImageImpl>(handle); }
void Image::unpin(void *handle) noexcept {
  unpin_void_handle<ImageImpl>(handle);
}

ImageType Image::type() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->type;
}

Format Image::format() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->format;
}

uvec3 Image::extent() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->extent;
}

uint32_t Image::mip_levels() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->mip_levels;
}

uint32_t Image::arrayLayers() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->arrayLayers;
}

SampleCount Image::samples() const noexcept {
  auto *impl = void_handle_ptr<ImageImpl>(m_handle);
  return impl->samples;
}

} // namespace strobe::rhi
