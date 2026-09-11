#pragma once

#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Buffer creation information.
 *
 * Describes the size, usage, and memory placement of a buffer.
 */
struct BufferInfo {
  /** Buffer size in bytes. */
  uint64_t size = 0;

  /** Intended buffer usages. */
  BufferUsage bufferUsage = BufferUsage::none;

  /** Requested memory usage. */
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::rhi
