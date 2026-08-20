#pragma once

#include <cstdint>
namespace strobe::gpu {

enum class MemoryGranularityClass : uint8_t {
  linear,  // Buffers and linear-tiled images
  optimal, // Optimal-tiled images
};

}
