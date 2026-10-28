#pragma once

#include "strobe/flow/inttypes.hpp"
#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
namespace strobe::rdg {

struct BufferInfo {
  flow::u64 size;
  rhi::BufferUsage bufferUsage = rhi::BufferUsage::none;
  rhi::MemoryUsage memoryUsage = rhi::MemoryUsage::automatic;
};

} // namespace strobe::rdg
