#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/image_view_type.hpp"
#include <limits>
#include <variant>

namespace strobe::rhi {

struct StorageBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};

struct StorageTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};

struct UniformBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};

struct UniformTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};

struct SampledImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none; // none => inherit from image.
  Format format = Format::undefined; // undefined => inherit from image.
  ImageSubresourceRange subresource{};
};

struct StorageImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none; // none => inherit from image.
  Format format = Format::undefined; // undefined => inherit from image.
  ImageSubresourceRange subresource{};
};

using ResourceDescriptorInfo =
    std::variant<StorageBufferDescriptorInfo, StorageTexelBufferDescriptorInfo,
                 UniformBufferDescriptorInfo, UniformTexelBufferDescriptorInfo,
                 SampledImageDescriptorInfo,
                 StorageImageDescriptorInfo>;

} // namespace strobe::rhi
