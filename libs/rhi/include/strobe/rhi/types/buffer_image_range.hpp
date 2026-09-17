#pragma once

#include "strobe/rhi/objects/buffer.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief buffer image range
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} buffer_image_range.hpp BufferImageRange
 */
// [BufferImageRange]
struct BufferImageRange {
  Buffer buffer{};
  uint64_t offset = 0;
  uint32_t rowLength = 0;
  uint32_t imageHeight = 0;
};
// [BufferImageRange]

} // namespace strobe::rhi
