#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Resource descriptor type.
 *
 * Specifies the type of resource represented by a descriptor.
 */
enum class DescriptorType {
  storage_buffer, ///< Storage buffer descriptor.
  uniform_buffer, ///< Uniform buffer descriptor.
};

} // namespace strobe::rhi
