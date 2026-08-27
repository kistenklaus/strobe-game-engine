#pragma once

#include <cstdint>

namespace strobe::rhi {

class Buffer;

// Buffer is implicitly convertible to a
// BufferOffset with offset = 0.
struct BufferOffset {
  const Buffer &buffer;
  uint64_t offset = 0;
};

} // namespace strobe::rhi

#include "strobe/rhi/objects/buffer.hpp"
