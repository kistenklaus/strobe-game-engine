#pragma once

#include "strobe/rhi/types/vertex_input_rate.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex buffer binding.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} vertex_binding.hpp VertexBinding
 *
 * Describes how elements are read from a bound vertex buffer.
 */
// [VertexBinding]
struct VertexBinding {
  uint32_t binding = 0;
  uint32_t stride = 0;
  VertexInputRate inputRate = VertexInputRate::vertex;
  uint32_t divisor = 1;
};
// [VertexBinding]

} // namespace strobe::rhi
