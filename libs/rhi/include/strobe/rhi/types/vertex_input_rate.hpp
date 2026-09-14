#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex input rate.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} vertex_input_rate.hpp VertexInputRate
 *
 * Specifies how vertex buffer elements advance during drawing.
 */
// [VertexInputRate]
enum class VertexInputRate : uint8_t {
  vertex,
  instance,
};
// [VertexInputRate]

} // namespace strobe::rhi
