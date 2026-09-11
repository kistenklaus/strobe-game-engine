#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer byte range.
 *
 * Identifies a contiguous byte range within a buffer.
 */
struct BufferRange {
  /** Referenced buffer. */
  Buffer buffer;

  /** Byte offset to the beginning of the range. */
  uint64_t offset = 0;

  /** Size of the range in bytes. */
  uint64_t size = 0;
};

} // namespace strobe::rhi
