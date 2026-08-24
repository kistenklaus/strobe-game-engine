#pragma once

#include <cstdint>

namespace strobe::gpu {

enum class GeometryType : uint32_t {
  triangles,
  aabbs,
};

} // namespace strobe::gpu
