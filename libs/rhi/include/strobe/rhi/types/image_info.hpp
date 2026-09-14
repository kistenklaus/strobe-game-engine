#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_flags.hpp"
#include "strobe/rhi/types/image_type.hpp"
#include "strobe/rhi/types/image_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/types/sample_count.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_info.hpp ImageInfo
 *
 * Describes the dimensions, format, usage, sampling, and memory placement of
 * an image.
 */
// [ImageInfo]
struct ImageInfo {
  ImageType type = ImageType::image_2d;
  Format format = Format::rgba8_srgb;
  uvec3 extent = uvec3(0, 0, 0);
  uint32_t mip_levels = 1;
  uint32_t arrayLayers = 1;
  SampleCount samples = SampleCount::x1;
  bool linearTiling = false;
  ImageFlags flags = ImageFlags::none;
  ImageUsage imageUsage = ImageUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};
// [ImageInfo]

} // namespace strobe::rhi
