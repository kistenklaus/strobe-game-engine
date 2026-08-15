#pragma once

#include "strobe/gpu/device/image_aspect.hpp"
#include <limits>
namespace strobe::gpu {

static constexpr uint32_t REMAINING_MIP_LEVELS =
    std::numeric_limits<uint32_t>::max();

static constexpr uint32_t REMAINING_ARRAY_LAYERS =
    std::numeric_limits<uint32_t>::max();

struct ImageSubresourceRange {
  ImageAspect aspect;
  uint32_t baseMipLevel = 0;
  uint32_t levelCount = REMAINING_MIP_LEVELS;
  uint32_t baseArrayLayer = 0;
  uint32_t layerCount = REMAINING_ARRAY_LAYERS;
};

} // namespace strobe::gpu
