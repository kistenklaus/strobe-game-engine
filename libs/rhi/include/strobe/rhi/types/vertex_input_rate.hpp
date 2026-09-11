#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex input rate.
 *
 * Specifies how vertex buffer elements advance during drawing.
 */
enum class VertexInputRate : uint8_t {
  vertex,   ///< Advance once per vertex.
  instance, ///< Advance once per instance.
};

} // namespace strobe::rhi
