#pragma once

#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/memory_usage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief TLAS creation information.
 *
 * Describes the capacity, build behavior, and memory placement of a top-level
 * acceleration structure.
 */
struct TlasInfo {
  /** Acceleration structure build flags. */
  BuildFlags buildFlags = BuildFlags::none;

  /** Requested memory usage. */
  MemoryUsage memoryUsage = MemoryUsage::automatic;

  /** Maximum number of instances. */
  uint32_t instanceCount = 0;
};

} // namespace strobe::rhi
