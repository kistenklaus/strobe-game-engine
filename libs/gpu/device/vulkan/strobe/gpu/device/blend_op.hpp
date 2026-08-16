#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class BlendOp : uint8_t {
  add,
  subtract,
  reverse_subtract,
  min,
  max,
};

} // namespace strobe::gpu
