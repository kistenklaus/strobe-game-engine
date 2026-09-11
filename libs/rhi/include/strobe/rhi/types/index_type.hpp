#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Index element type.
 *
 * Specifies the integer format used by an index buffer.
 */
enum class IndexType {
  none,   ///< No index type specified.
  uint8,  ///< 8-bit unsigned integer indices.
  uint16, ///< 16-bit unsigned integer indices.
  uint32, ///< 32-bit unsigned integer indices.
};

} // namespace strobe::rhi
