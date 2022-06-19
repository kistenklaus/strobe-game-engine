#include "strb/apis/vulkan/Shader.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Shader::Shader(const Device& device, const strb::vector<char> sourceCode)
    : device(&device), sourceCode(sourceCode) {
  VkShaderModuleCreateInfo shaderCreateInfo;
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.pNext = nullptr;
  shaderCreateInfo.flags = 0;
  shaderCreateInfo.codeSize = sourceCode.size();
  shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());
  VkResult result =
      vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &this->shader);
  ASSERT_VKRESULT(result);
}

void Shader::destroy() {
  assert(device != nullptr);
  assert(this->shader != VK_NULL_HANDLE);
  vkDestroyShaderModule(*device, this->shader, nullptr);
  dexec(device = nullptr);
  dexec(this->shader = VK_NULL_HANDLE);
}

}  // namespace strb::vulkan
