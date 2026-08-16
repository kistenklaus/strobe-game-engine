#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class StencilOp : uint8_t {
  keep,
  zero,
  replace,
  increment_and_clamp,
  decrement_and_clamp,
  invert,
  increment_and_wrap,
  decrement_and_wrap,
};

} // namespace strobe::gpu
