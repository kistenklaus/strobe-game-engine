#pragma once

#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
namespace strobe::rhi {

struct TlasInfo {
  BuildFlags buildFlags = BuildFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
  uint32_t instanceCount = 0;
};

} // namespace strobe::rhi
