#pragma once

#include "strobe/gpu/device/memory_allocation_flags.hpp"
#include "strobe/gpu/device/memory_granularity_class.hpp"
#include "strobe/gpu/device/memory_usage.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

struct MemoryRequirements {
  uint64_t size;
  uint64_t alignment;
  uint32_t memoryTypeBits;

  MemoryGranularityClass granularityClass;
  MemoryAllocationFlags flags;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::gpu
