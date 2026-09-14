#pragma once

#include <cstdint>
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Memory lifetime interval.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} memory_lifetime.hpp MemoryLifetime
 *
 * Describes the inclusive lifetime interval of an allocation for transient
 * memory aliasing.
 */
// [MemoryLifetime]
struct MemoryLifetime {
  uint32_t begin = 0;
  uint32_t end = std::numeric_limits<uint32_t>::max();
};
// [MemoryLifetime]

} // namespace strobe::rhi
