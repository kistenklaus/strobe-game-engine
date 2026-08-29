#pragma once

#include <cstdint>
namespace strobe::rhi {

struct TlasInstance {
  float transform[3][4];     // object-to-world [row-major]
  uint32_t customIndex : 24; // user defined value
  uint32_t mask : 8;         // visibility mask
  uint32_t shaderBindingTableRecordOffset : 24;
  uint32_t flags : 8;
  uint64_t blasAddress;
};

} // namespace strobe::rhi
