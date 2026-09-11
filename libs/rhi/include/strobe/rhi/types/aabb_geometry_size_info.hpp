#pragma once

#include "strobe/rhi/types/geometry_flags.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief AABB geometry size.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} aabb_geometry_size_info.hpp AabbGeometrySizeInfo
 *
 * Describes the maximum AABB geometry requirements used to size a BLAS.
 */
// [AabbGeometrySizeInfo]
struct AabbGeometrySizeInfo {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxAabbs = 0;
};
// [AabbGeometrySizeInfo]

} // namespace strobe::rhi
