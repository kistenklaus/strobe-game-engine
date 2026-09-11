#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <optional>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex position data.
 *
 * Describes a strided array of vertex positions used for triangle geometry.
 */
struct VertexPositionData {
  /** Buffer containing vertex positions. */
  Buffer buffer{};

  /** Byte offset to the first position. */
  uint64_t offset = 0;

  /** Byte stride between consecutive positions. */
  uint64_t stride = 0;
};

/**
 * \ingroup rhi
 * \brief Geometry transform data.
 *
 * Describes an optional transform applied to triangle geometry during a BLAS
 * build.
 */
struct TransformData {
  /** Buffer containing the transform. */
  Buffer buffer{};

  /** Byte offset to the transform. */
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief Triangle index data.
 *
 * Describes indexed triangle geometry and its base vertex offset.
 */
struct IndexData {
  /** Buffer containing triangle indices. */
  Buffer buffer{};

  /** Byte offset to the first index. */
  uint64_t offset = 0;

  /** Base vertex offset applied to referenced indices. */
  uint32_t vertexOffset = 0;
};

/**
 * \ingroup rhi
 * \brief Triangle geometry data.
 *
 * Describes triangle geometry used as input for a BLAS build.
 */
struct TriangleGeometryData {
  /** Number of triangles in the geometry. */
  uint32_t triangleCount = 0;

  /** Vertex position data. */
  VertexPositionData positions{};

  /** Optional index data for indexed geometry. */
  std::optional<IndexData> indices = std::nullopt;

  /** Optional geometry transform. */
  std::optional<TransformData> transform = std::nullopt;
};

} // namespace strobe::rhi
