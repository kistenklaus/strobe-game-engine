#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image dimensionality.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_type.hpp ImageType
 *
 * Specifies the dimensionality of an image.
 */
// [ImageType]
enum class ImageType : uint8_t {
  image_1d,
  image_2d,
  image_3d,
};
// [ImageType]

} // namespace strobe::rhi
