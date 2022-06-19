#include "strb/apis/vulkan/VertexBuffer.hpp"

namespace strb::vulkan {

VertexBuffer::VertexBuffer(const Device& device) {
  uint32_t vertexCount = 3;
  uint32_t vertexSize = 2 * sizeof(float);

  VkVertexInputBindingDescription bindingDesc;
  bindingDesc.binding = 0;
  bindingDesc.stride = vertexSize;
  bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  VkVertexInputAttributeDescription posDesc = {};
  posDesc.binding = 0;
  posDesc.location = 0;
  posDesc.format = VK_FORMAT_R32G32_SFLOAT;
  posDesc.offset = 0;
  VkPipelineVertexInputStateCreateInfo inputInfo;
  inputInfo.vertexAttributeDescriptionCount = 1;
  inputInfo.pVertexAttributeDescriptions = &posDesc;
  inputInfo.vertexBindingDescriptionCount = 1;
  inputInfo.pVertexBindingDescriptions = &bindingDesc;

  VkBufferCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.flags = 0;
  createInfo.pNext = nullptr;
  createInfo.size = vertexCount * vertexSize;
  createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult result =
      vkCreateBuffer(*this->device, &createInfo, nullptr, &this->vertexBuffer);
  ASSERT_VKRESULT(result);

  vkGetBufferMemoryRequirements(*this->device, this->vertexBuffer,
                                &this->memory);
  // TODO allocation
}

void VertexBuffer::destroy() {
  vkDestroyBuffer(*this->device, this->vertexBuffer, nullptr);
}

}  // namespace strb::vulkan
