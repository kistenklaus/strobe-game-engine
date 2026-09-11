#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Comparison operation.
 *
 * Specifies the comparison function used by depth and stencil tests.
 */
enum class CompareOp : uint8_t {
  never,            ///< Comparison always fails.
  less,             ///< Passes when the source value is less.
  equal,            ///< Passes when both values are equal.
  less_or_equal,    ///< Passes when the source value is less or equal.
  greater,          ///< Passes when the source value is greater.
  not_equal,        ///< Passes when both values differ.
  greater_or_equal, ///< Passes when the source value is greater or equal.
  always,           ///< Comparison always passes.
};

} // namespace strobe::rhi
