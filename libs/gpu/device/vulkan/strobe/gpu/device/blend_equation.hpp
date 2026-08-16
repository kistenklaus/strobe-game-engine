#pragma once

#include "strobe/gpu/device/blend_factor.hpp"
#include "strobe/gpu/device/blend_op.hpp"
namespace strobe::gpu {

struct BlendEquation {
  BlendFactor srcColor;
  BlendFactor dstColor;
  BlendOp colorBlendOp;
  BlendFactor srcAlpha;
  BlendFactor dstAlpha;
  BlendOp alphaBlendOp;
};

} // namespace strobe::gpu
