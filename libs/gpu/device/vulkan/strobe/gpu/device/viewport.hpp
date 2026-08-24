#pragma once

#include "strobe/core/lina/vec.hpp"
namespace strobe::gpu {

struct Viewport {
  vec2 position{0.0f};
  vec2 extent{0.0f};
  float minDepth = 0.0f;
  float maxDepth = 1.0f;
};

} // namespace strobe::gpu
