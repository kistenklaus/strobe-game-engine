#include "strobe/gpu/vulkan/shader_module.hpp"
#include <stdexcept>

namespace strobe::gpu::vulkan {

ShaderModule create_shader_module(Context *context,
                                  const ShaderModuleInfo &info) {
  assert(context != nullptr);
  assert(!info.spv.empty());

  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = static_cast<uint32_t>(info.spv.size()),
      .pCode = info.spv.data(),
  };
  ShaderModule module;
  VkResult result = vkCreateShaderModule(
      context->device(), &createInfo, context->driver_alloc(), &module.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
  return module;
}
void destroy_shader_module(Context *context, ShaderModule module) noexcept {
  assert(context != nullptr);
  assert(module);
  vkDestroyShaderModule(context->device(), module.handle,
                        context->driver_alloc());
}

} // namespace strobe::gpu::vulkan
