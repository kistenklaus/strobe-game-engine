#pragma once

#include "strobe/gpu/device/vertex_input_rate.hpp"
#include <cstdint>
namespace strobe::gpu {

struct VertexBinding {
  uint32_t binding = 0;
  uint32_t stride = 0;
  VertexInputRate inputRate = VertexInputRate::vertex;
  uint32_t divisor = 1;
};

} // namespace strobe::gpu
