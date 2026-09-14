#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Comparison operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} compare_op.hpp CompareOp
 *
 * Specifies the comparison function used by depth and stencil tests.
 */
// [CompareOp]
enum class CompareOp : uint8_t {
  never,
  less,
  equal,
  less_or_equal,
  greater,
  not_equal,
  greater_or_equal,
  always,
};
// [CompareOp]

} // namespace strobe::rhi
