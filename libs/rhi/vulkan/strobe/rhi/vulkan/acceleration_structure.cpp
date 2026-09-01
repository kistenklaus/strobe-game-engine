#include "strobe/rhi/vulkan/acceleration_structure.hpp"

#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
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
      vulkan_error(result, "Failed to create acceleration structure");
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

} // namespace strobe::rhi
