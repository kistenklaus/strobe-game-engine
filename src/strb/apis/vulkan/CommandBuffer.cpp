#include "strb/apis/vulkan/CommandBuffer.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

void CommandBuffer::begin(uint32_t flag) {
  assert(this->commandBuffer != VK_NULL_HANDLE);
  // TODO small overhead could only be created once.
  VkCommandBufferBeginInfo commandBufferBeginInfo;
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.pNext = nullptr;
  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  commandBufferBeginInfo.pInheritanceInfo = nullptr;
  const VkResult result =
      vkBeginCommandBuffer(this->commandBuffer, &commandBufferBeginInfo);
  ASSERT_VKRESULT(result);
}

void CommandBuffer::end() {
  assert(this->commandBuffer != VK_NULL_HANDLE);
  const VkResult result = vkEndCommandBuffer(this->commandBuffer);
  ASSERT_VKRESULT(result);
}

} // namespace strb::vulkan
