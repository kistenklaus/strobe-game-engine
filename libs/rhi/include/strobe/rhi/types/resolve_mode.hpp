#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class ResolveMode : uint8_t {
  none,
  sample_zero,
  average,
  min,
  max,
};

} // namespace strobe::rhi
