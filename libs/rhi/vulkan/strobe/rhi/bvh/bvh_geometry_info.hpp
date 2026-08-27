#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/types/build_flags.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::rhi {

struct BvhGeometryInfo {
  BuildFlags buildFlags;
  VkDeviceSize scratchSize;
  Vector<uint32_t> maxPrimitiveCount;
  Vector<VkAccelerationStructureGeometryKHR> geometries;
  VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
  Vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
};

} // namespace strobe::rhi
