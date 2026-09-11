#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image dimensionality.
 *
 * Specifies the dimensionality of an image.
 */
enum class ImageType : uint8_t {
  image_1d, ///< One-dimensional image.
  image_2d, ///< Two-dimensional image.
  image_3d, ///< Three-dimensional image.
};

} // namespace strobe::rhi
