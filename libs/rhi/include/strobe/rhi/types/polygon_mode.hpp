#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Polygon rasterization mode.
 *
 * Specifies how polygons are rasterized.
 */
enum class PolygonMode : uint8_t {
  fill,  ///< Rasterize polygon interiors.
  line,  ///< Rasterize polygon edges.
  point, ///< Rasterize polygon vertices.
};

} // namespace strobe::rhi
