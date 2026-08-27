#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/format.hpp"

namespace strobe::rhi {

// fwd decl.
class Image;

class ImageView : Object<ImageView> {
  friend class Device;
  friend class Image;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  ImageView() noexcept : Object(nullptr) {}
  ImageView(const ImageView &) noexcept;
  ImageView(ImageView &&) noexcept;
  ImageView &operator=(const ImageView &) noexcept;
  ImageView &operator=(ImageView &&) noexcept;
  ~ImageView() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  Format format() const noexcept;
  const Image &image() const noexcept;

  explicit ImageView(void *handle) noexcept : Object(handle) {}
};

} // namespace strobe::rhi

// complete fwd definition.
#include "strobe/rhi/objects/image.hpp"
