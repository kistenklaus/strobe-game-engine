#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

// Buffer is implicitly convertible to a
// BufferOffset with offset = 0.
struct BufferOffset {
  Buffer buffer{};
  uint64_t offset = 0;
};

} // namespace strobe::rhi
