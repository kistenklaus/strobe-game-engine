#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer byte range.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} buffer_range.hpp BufferRange
 *
 * Identifies a contiguous byte range within a buffer.
 */
// [BufferRange]
struct BufferRange {
  Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};
// [BufferRange]

} // namespace strobe::rhi
