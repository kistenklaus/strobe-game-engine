#pragma once

#include "strobe/rhi/memory/memory_allocation_flags.hpp"
#include "strobe/rhi/memory/memory_granularity_class.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct MemoryRequirements {
  uint64_t size;
  uint64_t alignment;
  uint32_t memoryTypeBits;

  MemoryGranularityClass granularityClass;
  MemoryAllocationFlags flags;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::rhi
