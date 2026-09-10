#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <optional>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct VertexPositionData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = 0;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct TransformData {
  Buffer buffer{};
  uint64_t offset = 0;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct IndexData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint32_t vertexOffset = 0;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct TriangleGeometryData {
  uint32_t triangleCount = 0;
  VertexPositionData positions{};
  std::optional<IndexData> indices = std::nullopt;
  std::optional<TransformData> transform = std::nullopt;
};

} // namespace strobe::rhi
