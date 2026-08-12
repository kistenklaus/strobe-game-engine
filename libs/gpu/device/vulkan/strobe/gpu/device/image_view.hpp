#pragma once

#include "strobe/gpu/device/format.hpp"
#include "strobe/gpu/device/image_aspect.hpp"
#include "strobe/gpu/device/image_view_type.hpp"
namespace strobe::gpu {

// fwd decl.
class Image;

struct ImageViewCreateInfo {
  ImageViewType type = ImageViewType::automatic;
  Format format = Format::undefined;
  ImageAspect aspect = ImageAspect::automatic;

  uint32_t baseMipLevel = 0;
  uint32_t mipLevelCount = UINT32_MAX;
  uint32_t baseArrayLayer = 0;
  uint32_t arrayLayerCount = UINT32_MAX;
};

class ImageView {
  friend class Device;
  friend class Image;

public:
  ImageView() noexcept : m_handle(nullptr) {}
  ImageView(const ImageView &) noexcept;
  ImageView(ImageView &&) noexcept;
  ImageView &operator=(const ImageView &) noexcept;
  ImageView &operator=(ImageView &&) noexcept;
  ~ImageView() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  Format format() const noexcept;
  Image image() const noexcept;

private:
  explicit ImageView(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
