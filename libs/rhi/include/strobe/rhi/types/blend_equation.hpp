#pragma once

#include "strobe/rhi/types/blend_factor.hpp"
#include "strobe/rhi/types/blend_op.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Color blend equation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet blend_equation.hpp BlendEquation
 *
 * Describes separate blending equations for color and alpha components.
 */
// [BlendEquation]
struct BlendEquation {
  BlendFactor srcColor;
  BlendFactor dstColor;
  BlendOp colorBlendOp;
  BlendFactor srcAlpha;
  BlendFactor dstAlpha;
  BlendOp alphaBlendOp;
};
// [BlendEquation]

} // namespace strobe::rhi
