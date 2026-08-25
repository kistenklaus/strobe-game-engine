#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <stdexcept>

namespace strobe::rhi::vulkan {

TimelineSemaphore create_timeline_semaphore(Context *context,
                                            const TimelineSemaphoreInfo &info) {
  assert(context != nullptr);
  assert(context->properties().timeline_semaphore);

  void *pNext = nullptr;
  VkSemaphoreTypeCreateInfo typeInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
      .pNext = pNext,
      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue = info.initalValue,
  };
  pNext = &typeInfo;
  VkSemaphoreCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = pNext,
      .flags = info.flags,
  };
  TimelineSemaphore sem;
  {
    ZoneScopedN("vkCreateSemaphore");
    VkResult result = vkCreateSemaphore(context->device(), &createInfo,
                                        context->driver_alloc(), &sem.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create timeline semaphore");
    }
  }
  return sem;
}

void destroy_timeline_semaphore(Context *context, TimelineSemaphore sem) {
  assert(context != nullptr);
  assert(context->properties().timeline_semaphore);
  assert(sem);
  ZoneScopedN("vkDestroySemaphore");
  vkDestroySemaphore(context->device(), sem.handle, context->driver_alloc());
}

uint64_t get_timeline_semaphore_value(Context *context, TimelineSemaphore sem) {
  assert(context != nullptr);
  assert(context->properties().timeline_semaphore);
  assert(sem);

  uint64_t value = 0;
  {
    ZoneScopedN("vkGetSemaphoreCounterValue");
    VkResult result =
        vkGetSemaphoreCounterValue(context->device(), sem.handle, &value);
    if (result != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to get timeline semaphore counter value");
    }
  }
  return value;
}

void signal_timeline_semaphore(Context *context, TimelineSemaphore sem,
                               uint64_t value) {
  assert(context != nullptr);
  assert(context->properties().timeline_semaphore);
  assert(sem);
  VkSemaphoreSignalInfo signalInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
      .pNext = nullptr,
      .semaphore = sem.handle,
      .value = value,
  };
  {
    ZoneScopedN("vkSignalSemaphore");
    VkResult result = vkSignalSemaphore(context->device(), &signalInfo);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to signal timeline semaphore");
    }
  }
}

bool wait_for_timeline_semaphore(Context *context, TimelineSemaphore sem,
                                 uint64_t value, uint64_t timeout) {
  VkSemaphoreWaitInfo waitInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .pNext = nullptr,
      .flags = 0,
      .semaphoreCount = 1,
      .pSemaphores = &sem.handle,
      .pValues = &value,
  };

  VkResult result;
  {
    ZoneScopedN("vkWaitSemaphores");
    result = vkWaitSemaphores(context->device(), &waitInfo, timeout);
  }
  if (result == VK_TIMEOUT) {
    return false;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to wait for timeline semaphore");
  }
  return true;
}

} // namespace strobe::rhi::vulkan
