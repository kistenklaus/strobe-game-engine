#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/aabb_geometry_size_info.hpp"
#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/types/triangle_geometry_size_info.hpp"
#include <variant>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief BLAS creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} blas_info.hpp BlasInfo
 *
 * Describes the geometry capacities and allocation properties used to create
 * a bottom-level acceleration structure.
 */
// [BlasInfo]
struct BlasInfo {
  BuildFlags buildFlags = BuildFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
  std::variant<span<const TriangleGeometrySizeInfo>,
               span<const AabbGeometrySizeInfo>>
      geometries = {};
};
// [BlasInfo]

} // namespace strobe::rhi
