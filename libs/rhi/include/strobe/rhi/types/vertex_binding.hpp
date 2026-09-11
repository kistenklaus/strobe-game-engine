#pragma once

#include "strobe/rhi/types/vertex_input_rate.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex buffer binding.
 *
 * Describes how elements are read from a bound vertex buffer.
 */
struct VertexBinding {
  /** Vertex buffer binding index. */
  uint32_t binding = 0;

  /** Byte stride between consecutive elements. */
  uint32_t stride = 0;

  /** Rate at which elements advance. */
  VertexInputRate inputRate = VertexInputRate::vertex;

  /** Number of draws between input advances for instance-rate bindings. */
  uint32_t divisor = 1;
};

} // namespace strobe::rhi
