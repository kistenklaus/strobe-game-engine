#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class BlendFactor : uint8_t {
  zero,
  one,

  src_color,
  one_minus_src_color,
  dst_color,
  one_minus_dst_color,

  src_alpha,
  one_minus_src_alpha,
  dst_alpha,
  one_minus_dst_alpha,

  constant_color,
  one_minus_constant_color,
  constant_alpha,
  one_minus_constant_alpha,

  src_alpha_saturate,

  src1_color,
  one_minus_src1_color,
  src1_alpha,
  one_minus_src1_alpha,
};

} // namespace strobe::rhi
