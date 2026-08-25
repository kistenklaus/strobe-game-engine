#pragma once

#include "strobe/rhi/types/blend_factor.hpp"
#include "strobe/rhi/types/blend_op.hpp"

namespace strobe::rhi {

struct BlendEquation {
  BlendFactor srcColor;
  BlendFactor dstColor;
  BlendOp colorBlendOp;
  BlendFactor srcAlpha;
  BlendFactor dstAlpha;
  BlendOp alphaBlendOp;
};

} // namespace strobe::rhi
