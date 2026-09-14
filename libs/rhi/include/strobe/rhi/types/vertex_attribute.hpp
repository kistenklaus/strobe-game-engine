#pragma once

#include "strobe/rhi/types/format.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex attribute description.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} vertex_attribute.hpp VertexAttribute
 *
 * Describes how a vertex attribute is read from a bound vertex buffer.
 */
// [VertexAttribute]
struct VertexAttribute {
  uint32_t location;
  uint32_t binding;
  Format format;
  uint32_t offset;
};
// [VertexAttribute]

} // namespace strobe::rhi
