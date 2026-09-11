#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image view type.
 *
 * Specifies how an image is interpreted through an image view.
 */
enum class ImageViewType : uint8_t {
  none,           ///< No image view type specified.
  image_1d,       ///< One-dimensional image view.
  image_2d,       ///< Two-dimensional image view.
  image_3d,       ///< Three-dimensional image view.
  image_1d_array, ///< One-dimensional image array view.
  image_2d_array, ///< Two-dimensional image array view.
  cube,           ///< Cube image view.
  cube_array,     ///< Cube image array view.
};

} // namespace strobe::rhi
