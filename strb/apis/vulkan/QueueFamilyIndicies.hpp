#pragma once
#include "strb/stl.hpp"

namespace strb::vulkan {

struct QueueFamilyIndicies {
  strb::optional<uint32_t> graphicsIndex;
  strb::optional<uint32_t> computeIndex;
  strb::optional<uint32_t> transferIndex;
};

}  // namespace strb::vulkan
