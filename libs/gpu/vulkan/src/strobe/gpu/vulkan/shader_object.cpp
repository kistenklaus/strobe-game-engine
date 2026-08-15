#include "strobe/gpu/vulkan/shader_object.hpp"
#include "strobe/gpu/vulkan/context/shader_obj.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

ShaderObject create_shader_object(Context *context,
                                  const ShaderObjectCreateInfo &info) {

  VkShaderCreateInfoEXT createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = info.flags,
      .stage = info.stage,
      .nextStage = info.nextStage,
      .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
      .codeSize = info.spirv.size_bytes(),
      .pCode = info.spirv.data(),
      .pName = "main",
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount =
          static_cast<uint32_t>(info.pushConstantRange.size()),
      .pPushConstantRanges = info.pushConstantRange.data(),
      .pSpecializationInfo = info.specInfo,
  };
  ShaderObject obj;

  {
    ZoneScopedN("vkCreateShadersEXT");
    VkResult result =
        vk_create_shaders(context->shaderObjFunc(), context->device(), 1,
                          &createInfo, context->driver_alloc(), &obj.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create shader object");
    }
  }
  return obj;
}

void destroy_shader_object(Context *context, ShaderObject shader) noexcept {
  assert(shader);
  ZoneScopedN("vkDestroyShaderEXT");
  vk_destroy_shader(context->shaderObjFunc(), context->device(), shader.handle);
}

} // namespace strobe::gpu::vulkan
