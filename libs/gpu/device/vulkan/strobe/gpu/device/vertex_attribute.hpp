#pragma once

#include "strobe/gpu/device/format.hpp"
#include <cstdint>
namespace strobe::gpu {

struct VertexAttribute {
  uint32_t location;
  uint32_t binding;
  Format format;
  uint32_t offset;
};

} // namespace strobe::gpu
