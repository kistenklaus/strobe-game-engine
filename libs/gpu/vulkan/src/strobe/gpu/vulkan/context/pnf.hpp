#pragma once

#include "strobe/gpu/vulkan/context/context_properties.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

struct PNF_Functions {
  // Calibrated timestamps
  PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR
      getPhysicalDeviceCalibrateableTimeDomains = nullptr;
  PFN_vkGetCalibratedTimestampsKHR getCalibratedTimestamps = nullptr;

  // Shader objects
  PFN_vkCreateShadersEXT createShaders = nullptr;
  PFN_vkDestroyShaderEXT destroyShader = nullptr;
  PFN_vkCmdBindShadersEXT cmdBindShaders = nullptr;
  PFN_vkGetShaderBinaryDataEXT getShaderBinaryData = nullptr;

  // Dynamic state provided by VK_EXT_shader_object
  PFN_vkCmdSetVertexInputEXT cmdSetVertexInput = nullptr;
  PFN_vkCmdSetRasterizationSamplesEXT cmdSetRasterizationSamples = nullptr;
  PFN_vkCmdSetSampleMaskEXT cmdSetSampleMask = nullptr;
  PFN_vkCmdSetAlphaToCoverageEnableEXT cmdSetAlphaToCoverageEnable = nullptr;
  PFN_vkCmdSetPolygonModeEXT cmdSetPolygonMode = nullptr;
  PFN_vkCmdSetDepthClampEnableEXT cmdSetDepthClampEnable = nullptr;
  PFN_vkCmdSetLogicOpEnableEXT cmdSetLogicOpEnable = nullptr;
  PFN_vkCmdSetLogicOpEXT cmdSetLogicOp = nullptr;
  PFN_vkCmdSetColorBlendEnableEXT cmdSetColorBlendEnable = nullptr;
  PFN_vkCmdSetColorBlendEquationEXT cmdSetColorBlendEquation = nullptr;
  PFN_vkCmdSetColorWriteMaskEXT cmdSetColorWriteMask = nullptr;
  PFN_vkCmdSetAlphaToOneEnableEXT cmdSetAlphaToOneEnable = nullptr;
  PFN_vkCmdSetPatchControlPointsEXT cmdSetPatchControlPoints = nullptr;

  [[nodiscard]]
  bool available() const noexcept {
    return createShaders != nullptr && destroyShader != nullptr &&
           cmdBindShaders != nullptr && getShaderBinaryData != nullptr &&
           cmdSetVertexInput != nullptr &&
           cmdSetRasterizationSamples != nullptr &&
           cmdSetSampleMask != nullptr &&
           cmdSetAlphaToCoverageEnable != nullptr &&
           cmdSetPolygonMode != nullptr && cmdSetDepthClampEnable != nullptr &&
           cmdSetLogicOpEnable != nullptr && cmdSetLogicOp != nullptr &&
           cmdSetColorBlendEnable != nullptr &&
           cmdSetColorBlendEquation != nullptr &&
           cmdSetColorWriteMask != nullptr &&
           cmdSetAlphaToOneEnable != nullptr &&
           cmdSetPatchControlPoints != nullptr;
  }

  [[nodiscard]]
  bool calibrated_timestamps_available() const noexcept {
    return getPhysicalDeviceCalibrateableTimeDomains != nullptr &&
           getCalibratedTimestamps != nullptr;
  }
};

[[nodiscard]]
inline PNF_Functions
load_pnf_functions(VkInstance instance, VkDevice device,
                   const ContextProperties &props) noexcept {
  assert(device != VK_NULL_HANDLE);

  PNF_Functions functions{};

  if (props.calibratedTimestamps) {
    functions.getPhysicalDeviceCalibrateableTimeDomains =
        reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR>(
            vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR"));

    functions.getCalibratedTimestamps =
        reinterpret_cast<PFN_vkGetCalibratedTimestampsKHR>(
            vkGetDeviceProcAddr(device, "vkGetCalibratedTimestampsKHR"));
  }

  if (props.shaderObjects) {
    functions.createShaders = reinterpret_cast<PFN_vkCreateShadersEXT>(
        vkGetDeviceProcAddr(device, "vkCreateShadersEXT"));

    functions.destroyShader = reinterpret_cast<PFN_vkDestroyShaderEXT>(
        vkGetDeviceProcAddr(device, "vkDestroyShaderEXT"));

    functions.cmdBindShaders = reinterpret_cast<PFN_vkCmdBindShadersEXT>(
        vkGetDeviceProcAddr(device, "vkCmdBindShadersEXT"));

    functions.getShaderBinaryData =
        reinterpret_cast<PFN_vkGetShaderBinaryDataEXT>(
            vkGetDeviceProcAddr(device, "vkGetShaderBinaryDataEXT"));

    functions.cmdSetVertexInput = reinterpret_cast<PFN_vkCmdSetVertexInputEXT>(
        vkGetDeviceProcAddr(device, "vkCmdSetVertexInputEXT"));

    functions.cmdSetRasterizationSamples =
        reinterpret_cast<PFN_vkCmdSetRasterizationSamplesEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT"));

    functions.cmdSetSampleMask = reinterpret_cast<PFN_vkCmdSetSampleMaskEXT>(
        vkGetDeviceProcAddr(device, "vkCmdSetSampleMaskEXT"));

    functions.cmdSetAlphaToCoverageEnable =
        reinterpret_cast<PFN_vkCmdSetAlphaToCoverageEnableEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetAlphaToCoverageEnableEXT"));

    functions.cmdSetPolygonMode = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(
        vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT"));

    functions.cmdSetDepthClampEnable =
        reinterpret_cast<PFN_vkCmdSetDepthClampEnableEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetDepthClampEnableEXT"));

    functions.cmdSetLogicOpEnable =
        reinterpret_cast<PFN_vkCmdSetLogicOpEnableEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEnableEXT"));

    functions.cmdSetLogicOp = reinterpret_cast<PFN_vkCmdSetLogicOpEXT>(
        vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEXT"));

    functions.cmdSetColorBlendEnable =
        reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT"));

    functions.cmdSetColorBlendEquation =
        reinterpret_cast<PFN_vkCmdSetColorBlendEquationEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEquationEXT"));

    functions.cmdSetColorWriteMask =
        reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetColorWriteMaskEXT"));

    functions.cmdSetAlphaToOneEnable =
        reinterpret_cast<PFN_vkCmdSetAlphaToOneEnableEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetAlphaToOneEnableEXT"));

    functions.cmdSetPatchControlPoints =
        reinterpret_cast<PFN_vkCmdSetPatchControlPointsEXT>(
            vkGetDeviceProcAddr(device, "vkCmdSetPatchControlPointsEXT"));
  }

  return functions;
}

// -----------------------------------------------------------------------------
// Shader creation
// -----------------------------------------------------------------------------

inline VkResult vk_create_shaders(const PNF_Functions *functions,
                                  VkDevice device, uint32_t createInfoCount,
                                  const VkShaderCreateInfoEXT *createInfos,
                                  const VkAllocationCallbacks *allocator,
                                  VkShaderEXT *shaders) noexcept {

  assert(functions != nullptr);
  assert(functions->createShaders != nullptr);

  return functions->createShaders(device, createInfoCount, createInfos,
                                  allocator, shaders);
}

// -----------------------------------------------------------------------------
// Shader destruction
// -----------------------------------------------------------------------------

inline void
vk_destroy_shader(const PNF_Functions *functions, VkDevice device,
                  VkShaderEXT shader,
                  const VkAllocationCallbacks *allocator = nullptr) noexcept {

  assert(functions != nullptr);
  assert(functions->destroyShader != nullptr);

  functions->destroyShader(device, shader, allocator);
}

// -----------------------------------------------------------------------------
// Shader binding
// -----------------------------------------------------------------------------

inline void vk_cmd_bind_shaders(const PNF_Functions *functions,
                                VkCommandBuffer commandBuffer,
                                uint32_t stageCount,
                                const VkShaderStageFlagBits *stages,
                                const VkShaderEXT *shaders) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdBindShaders != nullptr);

  functions->cmdBindShaders(commandBuffer, stageCount, stages, shaders);
}

// -----------------------------------------------------------------------------
// Shader binary retrieval
// -----------------------------------------------------------------------------

inline VkResult vk_get_shader_binary_data(const PNF_Functions *functions,
                                          VkDevice device, VkShaderEXT shader,
                                          size_t *dataSize,
                                          void *data) noexcept {

  assert(functions != nullptr);
  assert(functions->getShaderBinaryData != nullptr);

  return functions->getShaderBinaryData(device, shader, dataSize, data);
}

// -----------------------------------------------------------------------------
// Vertex input
// -----------------------------------------------------------------------------

inline void vk_cmd_set_vertex_input(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    uint32_t bindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *bindingDescriptions,
    uint32_t attributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT
        *attributeDescriptions) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetVertexInput != nullptr);

  functions->cmdSetVertexInput(commandBuffer, bindingDescriptionCount,
                               bindingDescriptions, attributeDescriptionCount,
                               attributeDescriptions);
}

// -----------------------------------------------------------------------------
// Rasterization samples
// -----------------------------------------------------------------------------

inline void
vk_cmd_set_rasterization_samples(const PNF_Functions *functions,
                                 VkCommandBuffer commandBuffer,
                                 VkSampleCountFlagBits samples) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetRasterizationSamples != nullptr);

  functions->cmdSetRasterizationSamples(commandBuffer, samples);
}

// -----------------------------------------------------------------------------
// Sample mask
// -----------------------------------------------------------------------------

inline void vk_cmd_set_sample_mask(const PNF_Functions *functions,
                                   VkCommandBuffer commandBuffer,
                                   VkSampleCountFlagBits samples,
                                   const VkSampleMask *sampleMask) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetSampleMask != nullptr);

  functions->cmdSetSampleMask(commandBuffer, samples, sampleMask);
}

// -----------------------------------------------------------------------------
// Alpha to coverage
// -----------------------------------------------------------------------------

inline void vk_cmd_set_alpha_to_coverage_enable(const PNF_Functions *functions,
                                                VkCommandBuffer commandBuffer,
                                                VkBool32 enable) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetAlphaToCoverageEnable != nullptr);

  functions->cmdSetAlphaToCoverageEnable(commandBuffer, enable);
}

// -----------------------------------------------------------------------------
// Polygon mode
// -----------------------------------------------------------------------------

inline void vk_cmd_set_polygon_mode(const PNF_Functions *functions,
                                    VkCommandBuffer commandBuffer,
                                    VkPolygonMode polygonMode) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetPolygonMode != nullptr);

  functions->cmdSetPolygonMode(commandBuffer, polygonMode);
}

// -----------------------------------------------------------------------------
// Depth clamp
// -----------------------------------------------------------------------------

inline void vk_cmd_set_depth_clamp_enable(const PNF_Functions *functions,
                                          VkCommandBuffer commandBuffer,
                                          VkBool32 enable) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetDepthClampEnable != nullptr);

  functions->cmdSetDepthClampEnable(commandBuffer, enable);
}

// -----------------------------------------------------------------------------
// Logic op enable
// -----------------------------------------------------------------------------

inline void vk_cmd_set_logic_op_enable(const PNF_Functions *functions,
                                       VkCommandBuffer commandBuffer,
                                       VkBool32 enable) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetLogicOpEnable != nullptr);

  functions->cmdSetLogicOpEnable(commandBuffer, enable);
}

// -----------------------------------------------------------------------------
// Logic op
// -----------------------------------------------------------------------------

inline void vk_cmd_set_logic_op(const PNF_Functions *functions,
                                VkCommandBuffer commandBuffer,
                                VkLogicOp logicOp) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetLogicOp != nullptr);

  functions->cmdSetLogicOp(commandBuffer, logicOp);
}

// -----------------------------------------------------------------------------
// Color blend enable
// -----------------------------------------------------------------------------

inline void vk_cmd_set_color_blend_enable(const PNF_Functions *functions,
                                          VkCommandBuffer commandBuffer,
                                          uint32_t firstAttachment,
                                          uint32_t attachmentCount,
                                          const VkBool32 *enables) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetColorBlendEnable != nullptr);

  functions->cmdSetColorBlendEnable(commandBuffer, firstAttachment,
                                    attachmentCount, enables);
}

// -----------------------------------------------------------------------------
// Color blend equation
// -----------------------------------------------------------------------------

inline void vk_cmd_set_color_blend_equation(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    uint32_t firstAttachment, uint32_t attachmentCount,
    const VkColorBlendEquationEXT *equations) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetColorBlendEquation != nullptr);

  functions->cmdSetColorBlendEquation(commandBuffer, firstAttachment,
                                      attachmentCount, equations);
}

// -----------------------------------------------------------------------------
// Color write mask
// -----------------------------------------------------------------------------

inline void
vk_cmd_set_color_write_mask(const PNF_Functions *functions,
                            VkCommandBuffer commandBuffer,
                            uint32_t firstAttachment, uint32_t attachmentCount,
                            const VkColorComponentFlags *writeMasks) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetColorWriteMask != nullptr);

  functions->cmdSetColorWriteMask(commandBuffer, firstAttachment,
                                  attachmentCount, writeMasks);
}

// -----------------------------------------------------------------------------
// Alpha to one
// -----------------------------------------------------------------------------

inline void vk_cmd_set_alpha_to_one_enable(const PNF_Functions *functions,
                                           VkCommandBuffer commandBuffer,
                                           VkBool32 enable) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetAlphaToOneEnable != nullptr);

  functions->cmdSetAlphaToOneEnable(commandBuffer, enable);
}

// -----------------------------------------------------------------------------
// Patch control points
// -----------------------------------------------------------------------------

inline void
vk_cmd_set_patch_control_points(const PNF_Functions *functions,
                                VkCommandBuffer commandBuffer,
                                uint32_t patchControlPoints) noexcept {

  assert(functions != nullptr);
  assert(functions->cmdSetPatchControlPoints != nullptr);

  functions->cmdSetPatchControlPoints(commandBuffer, patchControlPoints);
}

// -----------------------------------------------------------------------------
// Calibrated timestamp time domains
// -----------------------------------------------------------------------------

inline VkResult vk_get_physical_device_calibrateable_time_domains(
    const PNF_Functions *functions, VkPhysicalDevice physicalDevice,
    uint32_t *timeDomainCount, VkTimeDomainKHR *timeDomains) noexcept {

  assert(functions != nullptr);
  assert(functions->getPhysicalDeviceCalibrateableTimeDomains != nullptr);

  return functions->getPhysicalDeviceCalibrateableTimeDomains(
      physicalDevice, timeDomainCount, timeDomains);
}

// -----------------------------------------------------------------------------
// Calibrated timestamps
// -----------------------------------------------------------------------------

inline VkResult vk_get_calibrated_timestamps(
    const PNF_Functions *functions, VkDevice device, uint32_t timestampCount,
    const VkCalibratedTimestampInfoKHR *timestampInfos, uint64_t *timestamps,
    uint64_t *maxDeviation) noexcept {

  assert(functions != nullptr);
  assert(functions->getCalibratedTimestamps != nullptr);

  return functions->getCalibratedTimestamps(
      device, timestampCount, timestampInfos, timestamps, maxDeviation);
}

} // namespace strobe::gpu::vulkan
