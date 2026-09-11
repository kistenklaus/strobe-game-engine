#pragma once

#include "strobe/rhi/types/format.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex attribute description.
 *
 * Describes how a vertex attribute is read from a bound vertex buffer.
 */
struct VertexAttribute {
  /** Shader input location. */
  uint32_t location;

  /** Vertex buffer binding index. */
  uint32_t binding;

  /** Attribute data format. */
  Format format;

  /** Byte offset within each vertex element. */
  uint32_t offset;
};

} // namespace strobe::rhi
