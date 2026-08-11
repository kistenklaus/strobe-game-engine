#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct ShaderModule {
  VkShaderModule handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct ShaderModuleInfo {
  span<const uint32_t> spv;
};

ShaderModule create_shader_module(Context *context,
                                  const ShaderModuleInfo &info = {});

void destroy_shader_module(Context *context, ShaderModule module) noexcept;

} // namespace strobe::gpu::vulkan
