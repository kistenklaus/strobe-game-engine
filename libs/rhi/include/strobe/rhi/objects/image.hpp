#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_type.hpp"
#include "strobe/rhi/types/sample_count.hpp"

namespace strobe::rhi {

class Image : Object<Image> {
  friend class Device;
  friend struct ImageViewImpl;
  friend struct SwapchainImpl;
  friend struct SwapchainGenerationImpl;
  friend class CommandBuffer;
  friend class MemoryPool;

public:
  Image() noexcept : Object(nullptr) {}
  Image(const Image &) noexcept;
  Image(Image &&) noexcept;
  Image &operator=(const Image &) noexcept;
  Image &operator=(Image &&) noexcept;
  ~Image() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Image &lhs, const Image &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Image &lhs, const Image &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  ImageType type() const noexcept;
  Format format() const noexcept;
  uvec3 extent() const noexcept;
  uint32_t mip_levels() const noexcept;
  uint32_t arrayLayers() const noexcept;
  SampleCount samples() const noexcept;

  explicit Image(void *handle) noexcept : Object(handle) {}

private:
};

} // namespace strobe::rhi
