#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/aabb_geometry_info_size_info.hpp"
#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/types/triangle_geometry_size_info.hpp"
#include <variant>

namespace strobe::rhi {

struct BlasInfo {
  BuildFlags buildFlags = BuildFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
  std::variant<span<const TriangleGeometrySizeInfo>,
               span<const AabbGeometrySizeInfo>>
      geometries = {};
};

} // namespace strobe::rhi
