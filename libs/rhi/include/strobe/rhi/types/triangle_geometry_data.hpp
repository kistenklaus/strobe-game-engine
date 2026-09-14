#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <optional>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex position data.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} triangle_geometry_data.hpp VertexPositionData
 *
 * Describes a strided array of vertex positions used for triangle geometry.
 */
// [VertexPositionData]
struct VertexPositionData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = 0;
};
// [VertexPositionData]

/**
 * \ingroup rhi
 * \brief Geometry transform data.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} triangle_geometry_data.hpp TransformData
 *
 * Describes an optional transform applied to triangle geometry during a BLAS
 * build.
 */
// [TransformData]
struct TransformData {
  Buffer buffer{};
  uint64_t offset = 0;
};
// [TransformData]

/**
 * \ingroup rhi
 * \brief Triangle index data.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} triangle_geometry_data.hpp IndexData
 *
 * Describes indexed triangle geometry and its base vertex offset.
 */
// [IndexData]
struct IndexData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint32_t vertexOffset = 0;
};
// [IndexData]

/**
 * \ingroup rhi
 * \brief Triangle geometry data.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} triangle_geometry_data.hpp TriangleGeometryData
 *
 * Describes triangle geometry used as input for a BLAS build.
 */
// [TriangleGeometryData]
struct TriangleGeometryData {
  uint32_t triangleCount = 0;
  VertexPositionData positions{};
  std::optional<IndexData> indices = std::nullopt;
  std::optional<TransformData> transform = std::nullopt;
};
// [TriangleGeometryData]

} // namespace strobe::rhi
