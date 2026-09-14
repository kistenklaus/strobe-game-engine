#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image view type.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_view_type.hpp ImageViewType
 *
 * Specifies how an image is interpreted through an image view.
 */
// [ImageViewType]
enum class ImageViewType : uint8_t {
  none,
  image_1d,
  image_2d,
  image_3d,
  image_1d_array,
  image_2d_array,
  cube,
  cube_array,
};
// [ImageViewType]

} // namespace strobe::rhi
