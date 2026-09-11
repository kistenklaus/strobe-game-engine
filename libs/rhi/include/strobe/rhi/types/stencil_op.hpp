#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Stencil update operation.
 *
 * Specifies how a stencil value is modified after stencil and depth testing.
 */
enum class StencilOp : uint8_t {
  keep,                ///< Keep the current stencil value.
  zero,                ///< Set the stencil value to zero.
  replace,             ///< Replace the stencil value with the reference value.
  increment_and_clamp, ///< Increment and clamp to the maximum value.
  decrement_and_clamp, ///< Decrement and clamp to zero.
  invert,              ///< Bitwise invert the stencil value.
  increment_and_wrap,  ///< Increment and wrap on overflow.
  decrement_and_wrap,  ///< Decrement and wrap on underflow.
};

} // namespace strobe::rhi
