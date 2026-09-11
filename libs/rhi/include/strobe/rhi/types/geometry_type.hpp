#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Acceleration geometry type.
 *
 * Specifies the primitive representation used by acceleration structure
 * geometry.
 */
enum class GeometryType : uint32_t {
  triangles, ///< Triangle geometry.
  aabbs,     ///< Axis-aligned bounding box geometry.
};

} // namespace strobe::rhi
