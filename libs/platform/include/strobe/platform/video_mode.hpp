#pragma once

#include "strobe/core/lina/vec.hpp"
namespace strobe::platform {

struct VideoMode {
  uvec2 resolution;
  uint32_t refreshRate;
};

} // namespace strobe::platform
