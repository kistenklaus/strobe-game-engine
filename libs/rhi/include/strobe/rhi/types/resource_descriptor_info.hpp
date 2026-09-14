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
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp StorageBufferDescriptorInfo
 *
 * Describes a byte range of a buffer exposed as a storage resource.
 */
// [StorageBufferDescriptorInfo]
struct StorageBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};
// [StorageBufferDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Storage texel descriptor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp StorageTexelBufferDescriptorInfo
 *
 * Describes a formatted buffer range exposed as a storage texel resource.
 */
// [StorageTexelBufferDescriptorInfo]
struct StorageTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};
// [StorageTexelBufferDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Uniform buffer descriptor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp UniformBufferDescriptorInfo
 *
 * Describes a byte range of a buffer exposed as a uniform resource.
 */
// [UniformBufferDescriptorInfo]
struct UniformBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
};
// [UniformBufferDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Uniform texel descriptor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp UniformTexelBufferDescriptorInfo
 *
 * Describes a formatted buffer range exposed as a uniform texel resource.
 */
// [UniformTexelBufferDescriptorInfo]
struct UniformTexelBufferDescriptorInfo {
  Buffer buffer{};
  uint64_t size = std::numeric_limits<uint64_t>::max();
  uint64_t offset = 0;
  Format format = Format::undefined;
};
// [UniformTexelBufferDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Sampled image descriptor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp SampledImageDescriptorInfo
 *
 * Describes an image view exposed for sampled image access.
 */
// [SampledImageDescriptorInfo]
struct SampledImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none;
  Format format = Format::undefined;
  ImageSubresourceRange subresource{};
};
// [SampledImageDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Storage image descriptor.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp StorageImageDescriptorInfo
 *
 * Describes an image view exposed for storage image access.
 */
// [StorageImageDescriptorInfo]
struct StorageImageDescriptorInfo {
  Image image{};
  ImageLayout layout = ImageLayout::undefined;
  ImageViewType viewType = ImageViewType::none;
  Format format = Format::undefined;
  ImageSubresourceRange subresource{};
};
// [StorageImageDescriptorInfo]

/**
 * \ingroup rhi
 * \brief Resource descriptor information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} resource_descriptor_info.hpp ResourceDescriptorInfo
 *
 * Describes one resource descriptor stored in the device resource descriptor
 * heap.
 */
// [ResourceDescriptorInfo]
using ResourceDescriptorInfo =
    std::variant<StorageBufferDescriptorInfo, StorageTexelBufferDescriptorInfo,
                 UniformBufferDescriptorInfo, UniformTexelBufferDescriptorInfo,
                 SampledImageDescriptorInfo, StorageImageDescriptorInfo>;
// [ResourceDescriptorInfo]

} // namespace strobe::rhi
