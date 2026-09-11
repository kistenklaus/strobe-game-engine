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
 * \brief Storage buffer descriptor.
 *
 * Describes a byte range of a buffer exposed as a storage resource.
 */
struct StorageBufferDescriptorInfo {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Size of the exposed range in bytes. */
  uint64_t size = std::numeric_limits<uint64_t>::max();

  /** Byte offset to the beginning of the range. */
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief Storage texel descriptor.
 *
 * Describes a formatted buffer range exposed as a storage texel resource.
 */
struct StorageTexelBufferDescriptorInfo {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Size of the exposed range in bytes. */
  uint64_t size = std::numeric_limits<uint64_t>::max();

  /** Byte offset to the beginning of the range. */
  uint64_t offset = 0;

  /** Texel format. */
  Format format = Format::undefined;
};

/**
 * \ingroup rhi
 * \brief Uniform buffer descriptor.
 *
 * Describes a byte range of a buffer exposed as a uniform resource.
 */
struct UniformBufferDescriptorInfo {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Size of the exposed range in bytes. */
  uint64_t size = std::numeric_limits<uint64_t>::max();

  /** Byte offset to the beginning of the range. */
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief Uniform texel descriptor.
 *
 * Describes a formatted buffer range exposed as a uniform texel resource.
 */
struct UniformTexelBufferDescriptorInfo {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Size of the exposed range in bytes. */
  uint64_t size = std::numeric_limits<uint64_t>::max();

  /** Byte offset to the beginning of the range. */
  uint64_t offset = 0;

  /** Texel format. */
  Format format = Format::undefined;
};

/**
 * \ingroup rhi
 * \brief Sampled image descriptor.
 *
 * Describes an image view exposed for sampled image access.
 */
struct SampledImageDescriptorInfo {
  /** Referenced image. */
  Image image{};

  /** Image layout used for descriptor access. */
  ImageLayout layout = ImageLayout::undefined;

  /** Image view type, or ImageViewType::none to inherit from the image. */
  ImageViewType viewType = ImageViewType::none;

  /** View format, or Format::undefined to inherit from the image. */
  Format format = Format::undefined;

  /** Image subresources exposed by the descriptor. */
  ImageSubresourceRange subresource{};
};

/**
 * \ingroup rhi
 * \brief Storage image descriptor.
 *
 * Describes an image view exposed for storage image access.
 */
struct StorageImageDescriptorInfo {
  /** Referenced image. */
  Image image{};

  /** Image layout used for descriptor access. */
  ImageLayout layout = ImageLayout::undefined;

  /** Image view type, or ImageViewType::none to inherit from the image. */
  ImageViewType viewType = ImageViewType::none;

  /** View format, or Format::undefined to inherit from the image. */
  Format format = Format::undefined;

  /** Image subresources exposed by the descriptor. */
  ImageSubresourceRange subresource{};
};

/**
 * \ingroup rhi
 * \brief Resource descriptor information.
 *
 * Describes one resource descriptor stored in the device resource descriptor
 * heap.
 */
using ResourceDescriptorInfo =
    std::variant<StorageBufferDescriptorInfo, StorageTexelBufferDescriptorInfo,
                 UniformBufferDescriptorInfo, UniformTexelBufferDescriptorInfo,
                 SampledImageDescriptorInfo,
                 StorageImageDescriptorInfo>;

} // namespace strobe::rhi
