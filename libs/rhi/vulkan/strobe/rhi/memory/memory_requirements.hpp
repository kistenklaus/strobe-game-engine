#pragma once

#include "strobe/rhi/memory/memory_allocation_flags.hpp"
#include "strobe/rhi/memory/memory_granularity_class.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct MemoryRequirements {
  uint64_t size = 0;
  uint64_t alignment = 0;
  uint32_t memoryTypeBits = 0;

  MemoryGranularityClass granularityClass = MemoryGranularityClass::linear;
  MemoryAllocationFlags flags = MemoryAllocationFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;

  std::variant<std::monostate, vulkan::Buffer, vulkan::Image> dedicated{};
};

} // namespace strobe::rhi
