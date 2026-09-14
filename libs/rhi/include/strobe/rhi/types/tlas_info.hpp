#pragma once

#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/memory_usage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief TLAS creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} tlas_info.hpp TlasInfo
 *
 * Describes the capacity, build behavior, and memory placement of a top-level
 * acceleration structure.
 */
// [TlasInfo]
struct TlasInfo {
  BuildFlags buildFlags = BuildFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
  uint32_t instanceCount = 0;
};
// [TlasInfo]

} // namespace strobe::rhi
