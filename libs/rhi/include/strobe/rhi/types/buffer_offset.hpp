#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer byte offset.
 *
 * Identifies a byte position within a buffer.
 */
struct BufferOffset {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Byte offset into the buffer. */
  uint64_t offset = 0;
};

} // namespace strobe::rhi
