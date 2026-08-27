#pragma once

#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_flags.hpp"
#include "strobe/rhi/types/image_type.hpp"
#include "strobe/rhi/types/image_usage.hpp"
#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/types/sample_count.hpp"
namespace strobe::rhi {

struct ImageInfo {
  ImageType type = ImageType::image_2d;
  Format format = Format::rgba8_srgb;
  uvec3 extent = uvec3(0, 0, 0); // required!
  uint32_t mip_levels = 1;
  uint32_t arrayLayers = 1;
  SampleCount samples = SampleCount::x1;
  bool linearTiling = false;
  ImageFlags flags = ImageFlags::none;
  ImageUsage imageUsage = ImageUsage::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
};

} // namespace strobe::rhi
