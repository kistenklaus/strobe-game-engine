#pragma once

#include "strobe/rhi/types/image_aspect.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Remaining mip levels.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{cpp}
 * static constexpr uint32_t REMAINING_MIP_LEVELS = <implementation-defined>;
 * \endcode
 *
 * Selects all mip levels following the base mip level.
 */
static constexpr uint32_t REMAINING_MIP_LEVELS = std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Remaining array layers.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{cpp}
 * static constexpr uint32_t REMAINING_ARRAY_LAYERS = <implementation-defined>;
 * \endcode
 *
 * Selects all array layers following the base array layer.
 */
static constexpr uint32_t REMAINING_ARRAY_LAYERS =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Image subresource range.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_subresource_range.hpp ImageSubresourceRange
 *
 * Selects a range of image aspects, mip levels, and array layers.
 */
// [ImageSubresourceRange]
struct ImageSubresourceRange {
  ImageAspect aspect = ImageAspect::none;
  uint32_t baseMipLevel = 0;
  uint32_t levelCount = REMAINING_MIP_LEVELS;
  uint32_t baseArrayLayer = 0;
  uint32_t layerCount = REMAINING_ARRAY_LAYERS;
};
// [ImageSubresourceRange]

} // namespace strobe::rhi
