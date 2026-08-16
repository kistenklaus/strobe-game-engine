#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class VertexInputRate : uint8_t {
  vertex,
  instance,
};

} // namespace strobe::gpu
