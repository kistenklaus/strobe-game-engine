#pragma once

namespace strobe::gpu {

enum class MemoryUsage {
  automatic,
  device,
  mapped,
  mapped_write_sequential,
};

}
