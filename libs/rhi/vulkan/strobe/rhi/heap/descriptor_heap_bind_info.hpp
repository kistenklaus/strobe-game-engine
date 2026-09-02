#pragma once

#include "strobe/rhi/objects/buffer.hpp"
namespace strobe::rhi {

struct DescriptorHeapBindInfo {
  Buffer buffer;
  uint64_t size;
  uint64_t reservedOffset;
  uint64_t reservedSize;
};

} // namespace strobe::rhi
