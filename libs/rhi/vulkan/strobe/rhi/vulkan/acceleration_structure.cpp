#include "strobe/rhi/vulkan/acceleration_structure.hpp"

#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

vulkan::AccelerationStructure
vulkan::create_acceleration_structure(Context *context,
                                      const AccelerationStructureInfo &info) {

  VkAccelerationStructureCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .createFlags = 0,
      .buffer = info.buffer.handle,
      .offset = info.offset,
      .size = info.size,
      .type = info.type,
      .deviceAddress = 0,
  };

  AccelerationStructure accelerationStructure{};
  {
    ZoneScopedN("vkCreateAccelerationStructure");
    const VkResult result = vk_create_acceleration_structure(
        context->pnf(), context->device(), &createInfo, context->driver_alloc(),
        &accelerationStructure.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create acceleration structure");
    }
  }
  return accelerationStructure;
}
void vulkan::destroy_acceleration_structure(
    Context *context, AccelerationStructure accelerationStructure) noexcept {
  assert(context);
  assert(accelerationStructure);
  ZoneScopedN("vkDestroyAccelerationStructure");
  vk_destroy_acceleration_structure(context->pnf(), context->device(),
                                    accelerationStructure.handle);
}

VkDeviceAddress vulkan::get_acceleration_structure_device_address(
    Context *context, AccelerationStructure accelerationStructure) {
  VkAccelerationStructureDeviceAddressInfoKHR info{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
      .pNext = nullptr,
      .accelerationStructure = accelerationStructure.handle,
  };
  ZoneScopedN("vkGetAccelerationStructureDeviceAddress");
  return vk_get_acceleration_structure_device_address(context->pnf(),
                                                      context->device(), &info);
}
void vulkan::get_acceleration_structure_build_sizes(
    [[maybe_unused]] Context *context, const AccelerationStructureBuildDescription &buildDesc) {
  uint32_t geometryCount = 0;
  if (buildDesc.geometry.empty()) {
    assert(!buildDesc.pGeometry.empty());
    geometryCount = static_cast<uint32_t>(buildDesc.pGeometry.size());
  } else {
    assert(buildDesc.pGeometry.empty());
    geometryCount = static_cast<uint32_t>(buildDesc.geometry.size());
  }

  [[maybe_unused]] VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext = nullptr,
      .type = buildDesc.type,
      .flags = buildDesc.flags,
      .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount = geometryCount,
      .pGeometries = buildDesc.geometry.data(),
      .ppGeometries = buildDesc.pGeometry.data(),
      .scratchData =
          VkDeviceOrHostAddressKHR{
              .hostAddress = nullptr,
          },
  };

  // vk_get_acceleration_structure_build_sizes(
  //     context->pnf(), context->device(), buildDesc.type, &buildInfo,
  //     const uint32_t *maxPrimitiveCounts,
  //     VkAccelerationStructureBuildSizesInfoKHR *sizeInfo)
}

} // namespace strobe::rhi
