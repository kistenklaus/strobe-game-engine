#include "strobe/rhi/vulkan/shader_object.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

ShaderObject create_shader_object(Context *context,
                                  const ShaderObjectCreateInfo &info) {

  VkShaderCreateInfoEXT createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = info.flags | VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT,
      .stage = info.stage,
      .nextStage = info.nextStage,
      .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
      .codeSize = info.spirv.size_bytes(),
      .pCode = info.spirv.data(),
      .pName = "main",
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
      .pSpecializationInfo = info.specInfo,
  };
  ShaderObject obj;

  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCreateShadersEXT");
#endif
    VkResult result =
        vk_create_shaders(context->pnf(), context->device(), 1,
                          &createInfo, context->driver_alloc(), &obj.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create shader object");
    }
  }
  return obj;
}

void destroy_shader_object(Context *context, ShaderObject shader) noexcept {
  assert(shader);
#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkDestroyShaderEXT");
#endif
  vk_destroy_shader(context->pnf(), context->device(), shader.handle);
}

} // namespace strobe::rhi::vulkan
