#pragma once

#include "strobe/rhi/types/format.hpp"

namespace strobe::rhi {

// fwd decl.
class Image;

struct ImageViewCreateInfo {

  Format format = Format::undefined; // undefined => inherit from image

  uint32_t baseMipLevel = 0;
  uint32_t mipLevelCount = UINT32_MAX;
  uint32_t baseArrayLayer = 0;
  uint32_t arrayLayerCount = UINT32_MAX;
};

class ImageView {
  friend class Device;
  friend class Image;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  ImageView() noexcept : m_handle(nullptr) {}
  ImageView(const ImageView &) noexcept;
  ImageView(ImageView &&) noexcept;
  ImageView &operator=(const ImageView &) noexcept;
  ImageView &operator=(ImageView &&) noexcept;
  ~ImageView() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  Format format() const noexcept;
  const Image &image() const noexcept;

private:
  explicit ImageView(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::rhi

// complete fwd definition.
#include "strobe/rhi/objects/image.hpp"
