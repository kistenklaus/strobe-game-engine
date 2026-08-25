#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class BlendOp : uint8_t {
  add,
  subtract,
  reverse_subtract,
  min,
  max,
};

} // namespace strobe::rhi
