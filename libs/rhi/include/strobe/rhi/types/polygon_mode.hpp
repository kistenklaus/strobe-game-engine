#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Polygon rasterization mode.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} polygon_mode.hpp PolygonMode
 *
 * Specifies how polygons are rasterized.
 */
// [PolygonMode]
enum class PolygonMode : uint8_t {
  fill,
  line,
  point,
};
// [PolygonMode]

} // namespace strobe::rhi
