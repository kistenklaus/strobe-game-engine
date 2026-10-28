#pragma once

#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
namespace strobe::rdg {

struct BufferInfoImmediates {
  rhi::BufferUsage bufferUsage = rhi::BufferUsage::none;
  rhi::MemoryUsage memoryUsage = rhi::MemoryUsage::automatic;
};

} // namespace strobe::rdg
