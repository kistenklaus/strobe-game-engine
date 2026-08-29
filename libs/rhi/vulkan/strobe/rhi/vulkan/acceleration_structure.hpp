#pragma once

#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct AccelerationStructure {
  VkAccelerationStructureKHR handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct AccelerationStructureInfo {
  Buffer buffer = {};
  VkDeviceSize offset = 0;
  VkDeviceSize size = 0;
  VkAccelerationStructureTypeKHR type;
};

AccelerationStructure
create_acceleration_structure(Context *context,
                              const AccelerationStructureInfo &info);

void destroy_acceleration_structure(
    Context *context, AccelerationStructure accelerationStructure) noexcept;

VkDeviceAddress get_acceleration_structure_device_address(
    Context *context, AccelerationStructure accelerationStructure);

} // namespace strobe::rhi::vulkan
