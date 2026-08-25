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

struct AccelerationStructureGeometryTriangleDescription {
  VkFormat vertexFormat;
  VkDeviceAddress vertexData;
  VkDeviceSize vertexStride;
  uint32_t maxVertex;

  VkIndexType indexType;
  VkDeviceAddress indexData;

  VkDeviceAddress transformData;
};

struct AccelerationStructureGeometryAabbDescription {
  VkDeviceAddress data;
  VkDeviceSize stride;
};

struct AccelerationStructureGeometryInstanceDescription {
  VkBool32 arrayOfPointers;
  VkDeviceAddress data;
};

struct AccelerationStructureGeometryInfo {
  VkGeometryTypeKHR geometryType;
  VkAccelerationStructureGeometryDataKHR geometry;
  VkGeometryFlagsKHR flags;
};

struct AccelerationStructureBuildDescription {
  VkAccelerationStructureTypeKHR type;
  VkBuildAccelerationStructureFlagsKHR flags = 0;
  span<const VkAccelerationStructureGeometryKHR> geometry = {};
  span<const VkAccelerationStructureGeometryKHR *> pGeometry = {};
};

AccelerationStructure
create_acceleration_structure(Context *context,
                              const AccelerationStructureInfo &info);

void destroy_acceleration_structure(
    Context *context, AccelerationStructure accelerationStructure) noexcept;

VkDeviceAddress get_acceleration_structure_device_address(
    Context *context, AccelerationStructure accelerationStructure);

void get_acceleration_structure_build_sizes(
    Context *context, const AccelerationStructureBuildDescription &buildDesc);

} // namespace strobe::rhi::vulkan
