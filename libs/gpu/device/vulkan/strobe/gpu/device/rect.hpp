#pragma once

#include "strobe/core/lina/vec.hpp"

namespace strobe::gpu {

struct Rect {
  ivec2 offset = {0,0};
  uvec2 extent = {0,0};
};

}
