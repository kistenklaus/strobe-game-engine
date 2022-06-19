#include "strb/apis/vulkan/Semaphore.hpp"

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Semaphore::Semaphore(const Device& device) : device(&device) {
  VkSemaphoreCreateInfo semaphoreCreateInfo;
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreCreateInfo.pNext = nullptr;
  semaphoreCreateInfo.flags = 0;
  VkResult result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                                      &this->semaphore);
  ASSERT_VKRESULT(result);
}

void Semaphore::destroy() {
  vkDestroySemaphore(*this->device, this->semaphore, nullptr);
}

}  // namespace strb::vulkan
