#include "strb/apis/vulkan/CommandPool.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

CommandPool::CommandPool(const Device &device, const QueueFamily queueFamily)
    : device(&device), queueFamily(queueFamily) {
  VkCommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = nullptr;
  commandPoolCreateInfo.flags = 0;
  uint32_t queueFamilyIndex = [&]() {
    switch (queueFamily) {
    case QueueFamily::GRAPHICS:
      assert(device.getGraphicsQueueFamilyIndex().has_value());
      return device.getGraphicsQueueFamilyIndex().value();
    case QueueFamily::COMPUTE:
      assert(device.getComputeQueueFamilyIndex().has_value());
      return device.getComputeQueueFamilyIndex().value();
    case QueueFamily::TRANSFER:
      assert(device.getTransferQueueFamilyIndex().has_value());
      return device.getTransferQueueFamilyIndex().value();
    }
    throw strb::runtime_exception("unsupported QueueFamily");
  }();
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
  VkResult result = vkCreateCommandPool(*this->device, &commandPoolCreateInfo,
                                        nullptr, &commandPool);
  ASSERT_VKRESULT(result);
  assert(this->commandPool != VK_NULL_HANDLE);
}

void CommandPool::destroy() {
  this->free(this->allocatedBuffers);

  assert(this->device != nullptr);
  assert(this->commandPool != VK_NULL_HANDLE);
  vkDestroyCommandPool(*this->device, this->commandPool, nullptr);
  dexec(this->device = nullptr);
  dexec(this->commandPool = VK_NULL_HANDLE);
}

strb::vector<CommandBuffer>
CommandPool::allocate(const uint32_t commandBufferCount) {
  VkCommandBufferAllocateInfo commandBufferAllocateInfo;
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.pNext = nullptr;
  commandBufferAllocateInfo.commandPool = commandPool;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  commandBufferAllocateInfo.commandBufferCount = commandBufferCount;

  strb::vector<VkCommandBuffer> vkbuffers(commandBufferCount);
  VkResult result = vkAllocateCommandBuffers(
      *this->device, &commandBufferAllocateInfo, vkbuffers.data());
  // convert to CommandBuffer (overhead!)
  strb::vector<CommandBuffer> buffers(commandBufferCount);
  for (uint64_t i = 0; i < vkbuffers.size(); i++) {
    buffers[i] = CommandBuffer(vkbuffers[i]);
  }
  ASSERT_VKRESULT(result);
  // HINT: this is just for a saver cleanup, but it comes with a heavy overhead
  // if we allocate and free a lot of command buffers.
  for (const CommandBuffer &buffer : buffers) {
    this->allocatedBuffers.push_back(buffer);
  }
  return buffers;
}

void CommandPool::free(const strb::vector<CommandBuffer> &commandBuffers) {
  // convert to VkCommandBuffer vector (OVERHEAD!)
  if (commandBuffers.size() == 0)
    return;
  strb::vector<VkCommandBuffer> buffers(commandBuffers.size());
  for (uint64_t i = 0; i < buffers.size(); i++) {
    buffers[i] = commandBuffers[i];
    // for debugging set the vulkan handle to a null handle to make the
    // CommandBuffer instance useless.
    dexec(const_cast<CommandBuffer &>(commandBuffers[i]).commandBuffer =
              VK_NULL_HANDLE);
  }
  vkFreeCommandBuffers(*this->device, this->commandPool, buffers.size(),
                       buffers.data());
  // remove the command buffers from the locally stored command buffers.
  // HINT: this is just for a saver cleanup, but it comes with a heavy overhead
  // if we allocate and free a lot of command buffers.
  this->allocatedBuffers.erase(
      std::remove_if(this->allocatedBuffers.begin(),
                     this->allocatedBuffers.end(),
                     [&](CommandBuffer commandBuffer) {
                       return std::find(buffers.begin(), buffers.end(),
                                        commandBuffer) != buffers.end();
                     }),
      this->allocatedBuffers.end());
  // I HATE THIS!
}

void CommandPool::recreate() {
  this->free(this->allocatedBuffers);
  vkDestroyCommandPool(*this->device, this->commandPool, nullptr);

  VkCommandPoolCreateInfo commandPoolCreateInfo;
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.pNext = nullptr;
  commandPoolCreateInfo.flags = 0;
  uint32_t queueFamilyIndex = [&]() {
    switch (queueFamily) {
    case QueueFamily::GRAPHICS:
      assert(this->device->getGraphicsQueueFamilyIndex().has_value());
      return this->device->getGraphicsQueueFamilyIndex().value();
    case QueueFamily::COMPUTE:
      assert(this->device->getComputeQueueFamilyIndex().has_value());
      return this->device->getComputeQueueFamilyIndex().value();
    case QueueFamily::TRANSFER:
      assert(this->device->getTransferQueueFamilyIndex().has_value());
      return this->device->getTransferQueueFamilyIndex().value();
    }
    throw strb::runtime_exception("unsupported QueueFamily");
  }();
  commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
  VkResult result = vkCreateCommandPool(*this->device, &commandPoolCreateInfo,
                                        nullptr, &commandPool);
  ASSERT_VKRESULT(result);
  assert(this->commandPool != VK_NULL_HANDLE);
}

} // namespace strb::vulkan
