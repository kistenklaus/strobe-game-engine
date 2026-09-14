#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Resource descriptor type.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} descriptor_type.hpp DescriptorType
 *
 * Specifies the type of resource represented by a descriptor.
 */
// [DescriptorType]
enum class DescriptorType {
  storage_buffer, ///< Storage buffer descriptor.
  uniform_buffer, ///< Uniform buffer descriptor.
};
// [DescriptorType]

} // namespace strobe::rhi
