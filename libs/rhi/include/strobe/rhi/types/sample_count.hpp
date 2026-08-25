#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class SampleCount : uint8_t {
  x1  = 1,
  x2  = 2,
  x4  = 4,
  x8  = 8,
  x16 = 16,
  x32 = 32,
  x64 = 64,
};

}
