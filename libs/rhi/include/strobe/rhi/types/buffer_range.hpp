#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

struct BufferRange {
  Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};

} // namespace strobe::rhi
