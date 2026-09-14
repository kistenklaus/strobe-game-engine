#pragma once

#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} buffer_info.hpp BufferInfo
 *
 * Describes the size, usage, and memory placement of a buffer.
 */
// [BufferInfo]
struct BufferInfo {
  uint64_t size = 0;
  BufferUsage bufferUsage = BufferUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};
// [BufferInfo]

} // namespace strobe::rhi
