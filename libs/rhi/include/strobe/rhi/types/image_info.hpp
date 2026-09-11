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
 *
 * Describes the dimensions, format, usage, sampling, and memory placement of
 * an image.
 */
struct ImageInfo {
  /** Image dimensionality. */
  ImageType type = ImageType::image_2d;

  /** Image format. */
  Format format = Format::rgba8_srgb;

  /** Image extent in texels. */
  uvec3 extent = uvec3(0, 0, 0);

  /** Number of mip levels. */
  uint32_t mip_levels = 1;

  /** Number of array layers. */
  uint32_t arrayLayers = 1;

  /** Image sample count. */
  SampleCount samples = SampleCount::x1;

  /** Whether linear tiling is requested. */
  bool linearTiling = false;

  /** Additional image creation flags. */
  ImageFlags flags = ImageFlags::none;

  /** Intended image usages. */
  ImageUsage imageUsage = ImageUsage::none;

  /** Requested memory usage. */
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::rhi
