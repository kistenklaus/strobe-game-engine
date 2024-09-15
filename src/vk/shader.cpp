#include "./shader.h"

strobe::vk::ShaderModule strobe::vk::ShaderModule::create(
    const std::span<char>& code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
  VkShaderModule handle;
  if (vkCreateShaderModule(g_device, &createInfo, nullptr, &handle) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module.");
  }
  return ShaderModule(handle);
}
strobe::vk::ShaderModule::~ShaderModule() {
  if (m_handle != VK_NULL_HANDLE) {
    vkDestroyShaderModule(g_device, m_handle, nullptr);
  }
}

