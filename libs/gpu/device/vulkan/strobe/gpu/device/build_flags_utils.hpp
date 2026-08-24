#pragma once

#include "strobe/gpu/device/build_flags.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkBuildAccelerationStructureFlagsKHR
to_vk_build_flags(
    BuildFlags flags) noexcept {
  VkBuildAccelerationStructureFlagsKHR result = 0;

  if ((flags & BuildFlags::allow_update) != 0) {
    result |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
  }

  if ((flags & BuildFlags::allow_compaction) != 0) {
    result |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
  }

  if ((flags & BuildFlags::prefer_fast_trace) != 0) {
    result |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
  }

  if ((flags & BuildFlags::prefer_fast_build) != 0) {
    result |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
  }

  if ((flags & BuildFlags::low_memory) != 0) {
    result |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
  }

  return result;
}

static inline BuildFlags
from_vk_acceleration_structure_build_flags(
    VkBuildAccelerationStructureFlagsKHR flags) noexcept {
  BuildFlags result =
      BuildFlags::none;

  if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR) != 0) {
    result |= BuildFlags::allow_update;
  }

  if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR) != 0) {
    result |= BuildFlags::allow_compaction;
  }

  if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR) != 0) {
    result |= BuildFlags::prefer_fast_trace;
  }

  if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR) != 0) {
    result |= BuildFlags::prefer_fast_build;
  }

  if ((flags & VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR) != 0) {
    result |= BuildFlags::low_memory;
  }

  return result;
}

} // namespace strobe::gpu
