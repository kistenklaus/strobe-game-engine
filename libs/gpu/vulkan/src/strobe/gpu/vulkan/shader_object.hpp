#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>
namespace strobe::gpu::vulkan {

struct ShaderObjectCreateInfo {
  VkShaderStageFlagBits stage;

  VkShaderCreateFlagsEXT flags = 0;
  VkShaderStageFlags nextStage = 0;
  span<const uint32_t> spirv = {};
  span<const VkPushConstantRange> pushConstantRange = {};
  const VkSpecializationInfo *specInfo = nullptr;
};

struct ShaderObject {
  VkShaderEXT handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

ShaderObject create_shader_object(Context *context, const ShaderObjectCreateInfo& info);

void destroy_shader_object(Context *context, ShaderObject shader) noexcept;

} // namespace strobe::gpu::vulkan
