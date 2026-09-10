#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/geometry_flags.hpp"
#include "strobe/rhi/types/index_type.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
static constexpr uint32_t INFER_FROM_TRIANGLE_COUNT =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief todo
 */
struct TriangleGeometrySizeInfo {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxTriangles = 0;
  Format positionFormat = Format::rgb32_float;
  IndexType indexType = IndexType::none;
  uint32_t maxVertexIndex = INFER_FROM_TRIANGLE_COUNT;
  bool transform = false;
};

} // namespace strobe::rhi
