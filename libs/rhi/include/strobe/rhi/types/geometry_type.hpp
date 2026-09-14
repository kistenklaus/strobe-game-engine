#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Acceleration geometry type.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} geometry_type.hpp GeometryType
 *
 * Specifies the primitive representation used by acceleration structure
 * geometry.
 */
// [GeometryType]
enum class GeometryType : uint32_t {
  triangles,
  aabbs,
};
// [GeometryType]

} // namespace strobe::rhi
