#pragma once

#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include <cstdint>

namespace strobe::rhi {

struct BufferInfo {
  uint64_t size = 0;
  BufferUsage bufferUsage = BufferUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::rhi
