#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Primitive assembly topology.
 *
 * Specifies how input vertices are assembled into primitives for rasterization.
 */
enum class PrimitiveTopology : uint8_t {
  point_list,                    ///< Independent points.
  line_list,                     ///< Independent line segments.
  line_strip,                    ///< Connected line segments.
  triangle_list,                 ///< Independent triangles.
  triangle_strip,                ///< Connected triangle strip.
  triangle_fan,                  ///< Connected triangle fan.
  line_list_with_adjacency,      ///< Independent lines with adjacency data.
  line_strip_with_adjacency,     ///< Line strip with adjacency data.
  triangle_list_with_adjacency,  ///< Independent triangles with adjacency data.
  triangle_strip_with_adjacency, ///< Triangle strip with adjacency data.
  patch_list,                    ///< Patch control points.
};

} // namespace strobe::rhi
