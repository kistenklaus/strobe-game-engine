#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief sampler border color
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} border_color.hpp BorderColor
 */
// [BorderColor]
enum class BorderColor {
  float_transparent,
  int_transparent,
  float_opaque_black,
  int_opaque_black,
  float_opaque_white,
  int_opaque_white,
};
// [BorderColor]

}
