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

/**
 * \ingroup rhi
 * \brief todo
 */
struct StorageBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct StorageTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct UniformBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct UniformTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct SampledImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none; // none => inherit from image.
  Format format = Format::undefined; // undefined => inherit from image.
  ImageSubresourceRange subresource{};
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct StorageImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none; // none => inherit from image.
  Format format = Format::undefined; // undefined => inherit from image.
  ImageSubresourceRange subresource{};
};

/**
 * \ingroup rhi
 * \brief todo
 */
using ResourceDescriptorInfo =
    std::variant<StorageBufferDescriptorInfo, StorageTexelBufferDescriptorInfo,
                 UniformBufferDescriptorInfo, UniformTexelBufferDescriptorInfo,
                 SampledImageDescriptorInfo,
                 StorageImageDescriptorInfo>;

} // namespace strobe::rhi
