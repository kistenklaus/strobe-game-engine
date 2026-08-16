#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class ResolveMode : uint8_t {
  none,
  sample_zero,
  average,
  min,
  max,
};

} // namespace strobe::gpu
