#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/descriptor_type.hpp"
namespace strobe::rhi {

struct BufferDescriptorInfo {
  Buffer buffer;
  uint64_t offset;
  DescriptorType type = DescriptorType::storage_buffer;
};

} // namespace strobe::rhi
