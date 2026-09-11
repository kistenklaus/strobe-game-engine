#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/aabb.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief AABB geometry data.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} aabb_geometry_data.hpp AabbGeometryData
 *
 * Describes a strided array of Aabb elements used as input for a Blas build.
 */
// [AabbGeometryData]
struct AabbGeometryData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = sizeof(Aabb);
  uint32_t count = 0;
}; 
// [AabbGeometryData]

} // namespace strobe::rhi
