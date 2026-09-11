#pragma once

#include <cstdint>
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Memory lifetime interval.
 *
 * Describes the inclusive lifetime interval of an allocation for transient
 * memory aliasing.
 */
struct MemoryLifetime {
  /** First lifetime index at which the allocation is used. */
  uint32_t begin = 0;

  /** Last lifetime index at which the allocation is used. */
  uint32_t end = std::numeric_limits<uint32_t>::max();
};

} // namespace strobe::rhi
