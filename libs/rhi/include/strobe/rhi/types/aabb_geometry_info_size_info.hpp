#pragma once

#include "strobe/rhi/types/geometry_flags.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct AabbGeometrySizeInfo {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxAabbs = 0;
};

} // namespace strobe::rhi
