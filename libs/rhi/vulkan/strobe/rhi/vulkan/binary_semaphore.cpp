#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"

namespace strobe::rhi::vulkan {

BinarySemaphore create_binary_semaphore(Context *context,
                                        const SemaphoreInfo &info) {
  assert(context != nullptr);
  VkSemaphoreCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
  };
  BinarySemaphore sem;
  {
    ZoneScopedN("vkCreateSemaphore")
    VkResult result = vkCreateSemaphore(context->device(), &createInfo,
                                        context->driver_alloc(), &sem.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create semaphore");
    }
  }
  return sem;
}
void destroy_binary_semaphore(Context *context, BinarySemaphore sem) noexcept {
  assert(context != nullptr);
  assert(sem);
  ZoneScopedN("vkDestroySemaphore");
  vkDestroySemaphore(context->device(), sem.handle, context->driver_alloc());
}

} // namespace strobe::rhi::vulkan
