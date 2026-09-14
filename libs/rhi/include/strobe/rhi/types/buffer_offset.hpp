#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer byte offset.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} buffer_offset.hpp BufferOffset
 *
 * Identifies a byte position within a buffer.
 */
// [BufferOffset]
struct BufferOffset {
  /** Referenced buffer. */
  Buffer buffer{};

  /** Byte offset into the buffer. */
  uint64_t offset = 0;
};
// [BufferOffset]

} // namespace strobe::rhi
