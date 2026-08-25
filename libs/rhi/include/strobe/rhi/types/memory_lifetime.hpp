#pragma once

#include <cstdint>
#include <limits>

namespace strobe::rhi {

struct MemoryLifetime {
  uint32_t begin = 0;
  uint32_t end = std::numeric_limits<uint32_t>::max();
};

} // namespace strobe::rhi
