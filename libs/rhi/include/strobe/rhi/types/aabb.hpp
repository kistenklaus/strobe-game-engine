#pragma once

#include <cstddef>
#include <type_traits>

namespace strobe::rhi {

struct Aabb {
  float minX = 0;
  float minY = 0;
  float minZ = 0;
  float maxX = 0;
  float maxY = 0;
  float maxZ = 0;
};

static_assert(std::is_standard_layout_v<Aabb>);
static_assert(sizeof(Aabb) == 6 * sizeof(float));
static_assert(offsetof(Aabb, minX) == 0);
static_assert(offsetof(Aabb, minY) == 4);
static_assert(offsetof(Aabb, minZ) == 8);
static_assert(offsetof(Aabb, maxX) == 12);
static_assert(offsetof(Aabb, maxY) == 16);
static_assert(offsetof(Aabb, maxZ) == 20);

} // namespace strobe::rhi
