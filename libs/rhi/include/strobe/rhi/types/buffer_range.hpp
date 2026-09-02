#pragma once

#include <cstdint>

namespace strobe::rhi {

class Buffer;

// Buffer is implicitly convertible to a
// BufferOffset with offset = 0.
struct BufferRange {
  const Buffer &buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};

} // namespace strobe::rhi

#include "strobe/rhi/objects/buffer.hpp"
