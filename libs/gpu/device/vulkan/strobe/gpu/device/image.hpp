#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/format.hpp"
#include "strobe/gpu/device/image_aspect.hpp"
#include "strobe/gpu/device/image_flags.hpp"
#include "strobe/gpu/device/image_type.hpp"
#include "strobe/gpu/device/image_usage.hpp"
#include "strobe/gpu/device/image_view.hpp"
#include "strobe/gpu/device/image_view_type.hpp"
#include "strobe/gpu/device/memory_usage.hpp"
#include "strobe/gpu/device/sample_count.hpp"

namespace strobe::gpu {

struct ImageCreateInfo {
  ImageType type = ImageType::image_2d;
  Format format = Format::rgba8_srgb;
  uvec3 extent = uvec3(0, 0, 0); // required!
  uint32_t mip_levels = 1;
  uint32_t arrayLayers = 1;
  SampleCount samples = SampleCount::x1;
  bool linearTiling = false;
  ImageFlags flags = ImageFlags::none;
  ImageUsage usage = ImageUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

class Image {
  friend class Device;
  friend struct ImageViewImpl;
  friend struct SwapchainImpl;
  friend struct SwapchainGenerationImpl;
  friend class CommandBuffer;

public:
  Image() noexcept : m_handle(nullptr) {}
  Image(const Image &) noexcept;
  Image(Image &&) noexcept;
  Image &operator=(const Image &) noexcept;
  Image &operator=(Image &&) noexcept;
  ~Image() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  ImageType type() const noexcept;
  Format format() const noexcept;
  uvec3 extent() const noexcept;
  uint32_t mip_levels() const noexcept;
  uint32_t arrayLayers() const noexcept;
  SampleCount samples() const noexcept;

  ImageView create_view(ImageViewType type, ImageAspect aspect,
                        const ImageViewCreateInfo &createInfo = {});

private:
  Image(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
