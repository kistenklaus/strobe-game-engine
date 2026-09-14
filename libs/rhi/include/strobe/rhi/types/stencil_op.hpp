#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Stencil update operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} stencil_op.hpp StencilOp
 *
 * Specifies how a stencil value is modified after stencil and depth testing.
 */
// [StencilOp]
enum class StencilOp : uint8_t {
  keep,
  zero,
  replace,
  increment_and_clamp,
  decrement_and_clamp,
  invert,
  increment_and_wrap,
  decrement_and_wrap,
};
// [StencilOp]

} // namespace strobe::rhi
