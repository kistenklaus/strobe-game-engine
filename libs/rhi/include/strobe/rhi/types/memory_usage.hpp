#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class MemoryUsage : uint8_t {
  automatic,
  device,
  mapped,
  mapped_write_sequential,
  mapped_incoherent,
};

}
