#pragma once

#include <cstdint>

namespace strobe::rhi {

enum class PrimitiveTopology : uint8_t {
  point_list,
  line_list,
  line_strip,
  triangle_list,
  triangle_strip,
  triangle_fan,
  line_list_with_adjacency,
  line_strip_with_adjacency,
  triangle_list_with_adjacency,
  triangle_strip_with_adjacency,
  patch_list,
};

} // namespace strobe::rhi
