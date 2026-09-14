#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Index element type.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} index_type.hpp IndexType
 *
 * Specifies the integer format used by an index buffer.
 */
// [IndexType]
enum class IndexType {
  none,
  uint8,
  uint16,
  uint32,
};
// [IndexType]

} // namespace strobe::rhi
