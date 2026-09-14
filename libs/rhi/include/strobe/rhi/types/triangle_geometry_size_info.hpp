#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/geometry_flags.hpp"
#include "strobe/rhi/types/index_type.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Infer maximum vertex index.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{cpp}
 * static constexpr uint32_t INFER_FROM_TRIANGLE_COUNT = <sentinal>;
 * \endcode
 *
 * Infers the maximum vertex index from the configured triangle count.
 */
static constexpr uint32_t INFER_FROM_TRIANGLE_COUNT =
    std::numeric_limits<uint32_t>::max();

/**
 * \ingroup rhi
 * \brief Triangle geometry size.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} triangle_geometry_size_info.hpp TriangleGeometrySizeInfo
 *
 * Describes the maximum triangle geometry requirements used to size a BLAS.
 */
// [TriangleGeometrySizeInfo]
struct TriangleGeometrySizeInfo {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxTriangles = 0;
  Format positionFormat = Format::rgb32_float;
  IndexType indexType = IndexType::none;
  uint32_t maxVertexIndex = INFER_FROM_TRIANGLE_COUNT;
  bool transform = false;
};
// [TriangleGeometrySizeInfo]

} // namespace strobe::rhi
