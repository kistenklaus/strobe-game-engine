#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/aabb.hpp"

namespace strobe::rhi {

struct AabbGeometryData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = sizeof(Aabb);
  uint32_t count = 0; 
};

}
