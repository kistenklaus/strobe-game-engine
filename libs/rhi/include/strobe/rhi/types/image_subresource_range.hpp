#pragma once

#include "strobe/rhi/types/image_aspect.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Remaining mip levels.
 *
 * Selects all mip levels following the base mip level.
 */
static constexpr uint32_t REMAINING_MIP_LEVELS =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Remaining array layers.
 *
 * Selects all array layers following the base array layer.
 */
static constexpr uint32_t REMAINING_ARRAY_LAYERS =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Image subresource range.
 *
 * Selects a range of image aspects, mip levels, and array layers.
 */
struct ImageSubresourceRange {
  /** Selected image aspects. */
  ImageAspect aspect = ImageAspect::none;

  /** First selected mip level. */
  uint32_t baseMipLevel = 0;

  /** Number of selected mip levels. */
  uint32_t levelCount = REMAINING_MIP_LEVELS;

  /** First selected array layer. */
  uint32_t baseArrayLayer = 0;

  /** Number of selected array layers. */
  uint32_t layerCount = REMAINING_ARRAY_LAYERS;
};

} // namespace strobe::rhi
