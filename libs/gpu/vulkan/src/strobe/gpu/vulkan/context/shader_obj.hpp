#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

struct PNF_ShaderObjectFunctions {
  PFN_vkCreateShadersEXT createShaders = nullptr;
  PFN_vkDestroyShaderEXT destroyShader = nullptr;
  PFN_vkCmdBindShadersEXT cmdBindShaders = nullptr;
  PFN_vkGetShaderBinaryDataEXT getShaderBinaryData = nullptr;

  [[nodiscard]]
  bool available() const noexcept {
    return createShaders != nullptr && destroyShader != nullptr &&
           cmdBindShaders != nullptr && getShaderBinaryData != nullptr;
  }
};

[[nodiscard]]
inline PNF_ShaderObjectFunctions
load_shader_object_functions(VkDevice device) noexcept {
  assert(device != VK_NULL_HANDLE);

  PNF_ShaderObjectFunctions functions{};

  functions.createShaders = reinterpret_cast<PFN_vkCreateShadersEXT>(
      vkGetDeviceProcAddr(device, "vkCreateShadersEXT"));

  functions.destroyShader = reinterpret_cast<PFN_vkDestroyShaderEXT>(
      vkGetDeviceProcAddr(device, "vkDestroyShaderEXT"));

  functions.cmdBindShaders = reinterpret_cast<PFN_vkCmdBindShadersEXT>(
      vkGetDeviceProcAddr(device, "vkCmdBindShadersEXT"));

  functions.getShaderBinaryData =
      reinterpret_cast<PFN_vkGetShaderBinaryDataEXT>(
          vkGetDeviceProcAddr(device, "vkGetShaderBinaryDataEXT"));

  return functions;
}

// -----------------------------------------------------------------------------
// Shader creation
// -----------------------------------------------------------------------------

inline VkResult vk_create_shaders(const PNF_ShaderObjectFunctions *functions,
                                  VkDevice device, uint32_t createInfoCount,
                                  const VkShaderCreateInfoEXT *createInfos,
                                  const VkAllocationCallbacks *allocator,
                                  VkShaderEXT *shaders) noexcept {

  if (functions->createShaders == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  return functions->createShaders(device, createInfoCount, createInfos,
                                 allocator, shaders);
}

// -----------------------------------------------------------------------------
// Shader destruction
// -----------------------------------------------------------------------------

inline void
vk_destroy_shader(const PNF_ShaderObjectFunctions *functions, VkDevice device,
                  VkShaderEXT shader,
                  const VkAllocationCallbacks *allocator = nullptr) noexcept {

  assert(functions->destroyShader != nullptr);

  if (functions->destroyShader == nullptr) {
    return;
  }

  functions->destroyShader(device, shader, allocator);
}

// -----------------------------------------------------------------------------
// Shader binding
// -----------------------------------------------------------------------------

inline void vk_cmd_bind_shaders(const PNF_ShaderObjectFunctions *functions,
                                VkCommandBuffer commandBuffer,
                                uint32_t stageCount,
                                const VkShaderStageFlagBits *stages,
                                const VkShaderEXT *shaders) noexcept {

  assert(functions->cmdBindShaders != nullptr);

  if (functions->cmdBindShaders == nullptr) {
    return;
  }

  functions->cmdBindShaders(commandBuffer, stageCount, stages, shaders);
}

// -----------------------------------------------------------------------------
// Shader binary retrieval
// -----------------------------------------------------------------------------

inline VkResult
vk_get_shader_binary_data(const PNF_ShaderObjectFunctions *functions,
                          VkDevice device, VkShaderEXT shader, size_t *dataSize,
                          void *data) noexcept {

  if (functions->getShaderBinaryData == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  return functions->getShaderBinaryData(device, shader, dataSize, data);
}

} // namespace strobe::gpu::vulkan
