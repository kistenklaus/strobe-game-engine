#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include <fmt/base.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct AccelerationStructure {
  VkAccelerationStructureKHR handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct AccelerationStructureInfo {
  Buffer buffer = {};
  VkDeviceSize offset = 0;
  VkDeviceSize size = 0;
};

AccelerationStructure create_acceleration_structure(Context *context, const AccelerationStructureInfo& info = {}) {


  VkAccelerationStructureCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .createFlags = 0,
      .buffer = info.buffer.handle,
      .offset = info.offset,
      .size = info.size,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .deviceAddress = get_buffer_device_address(context, info.buffer),
  };

  AccelerationStructure ac{};
  VkResult result = vkCreateAccelerationStructureKHR(
      context->device(), &createInfo, context->driver_alloc(), &ac.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create acceleration structure");
  }

  return ac;
}

} // namespace strobe::gpu::vulkan
