#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/geometry_flags.hpp"
#include "strobe/rhi/types/index_type.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Infer maximum vertex index.
 *
 * Infers the maximum vertex index from the configured triangle count.
 */
static constexpr uint32_t INFER_FROM_TRIANGLE_COUNT =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Triangle geometry size.
 *
 * Describes the maximum triangle geometry requirements used to size a BLAS.
 */
struct TriangleGeometrySizeInfo {
  /** Geometry build flags. */
  GeometryFlags flags = GeometryFlags::none;

  /** Maximum number of triangles. */
  uint32_t maxTriangles = 0;

  /** Format of vertex position data. */
  Format positionFormat = Format::rgb32_float;

  /** Index element type, or IndexType::none for non-indexed geometry. */
  IndexType indexType = IndexType::none;

  /** Maximum referenced vertex index, or INFER_FROM_TRIANGLE_COUNT to infer it. */
  uint32_t maxVertexIndex = INFER_FROM_TRIANGLE_COUNT;

  /** Whether the geometry may provide a transform. */
  bool transform = false;
};

} // namespace strobe::rhi
