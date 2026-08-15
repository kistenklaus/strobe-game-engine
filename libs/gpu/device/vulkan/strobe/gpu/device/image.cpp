#include "strobe/gpu/device/image.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_impl.hpp"

namespace strobe::gpu {

Image::Image(const Image &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ImageImpl>(m_handle);
  }
}

Image::Image(Image &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Image &Image::operator=(const Image &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ImageImpl>(o.m_handle);
  }
  unpin_void_handle<ImageImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Image &Image::operator=(Image &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ImageImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Image::~Image() noexcept { unpin_void_handle<ImageImpl>(m_handle); }

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

} // namespace strobe::gpu
