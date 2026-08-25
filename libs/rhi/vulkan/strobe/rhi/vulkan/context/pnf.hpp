#pragma once

#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace strobe::rhi::vulkan {

struct PNF_Functions {
  // Deferred host operations
  PFN_vkCreateDeferredOperationKHR createDeferredOperation = nullptr;
  PFN_vkDestroyDeferredOperationKHR destroyDeferredOperation = nullptr;
  PFN_vkGetDeferredOperationMaxConcurrencyKHR
      getDeferredOperationMaxConcurrency = nullptr;
  PFN_vkGetDeferredOperationResultKHR getDeferredOperationResult = nullptr;
  PFN_vkDeferredOperationJoinKHR deferredOperationJoin = nullptr;

  // Acceleration structures
  PFN_vkCreateAccelerationStructureKHR createAccelerationStructure = nullptr;
  PFN_vkDestroyAccelerationStructureKHR destroyAccelerationStructure = nullptr;
  PFN_vkCmdBuildAccelerationStructuresKHR cmdBuildAccelerationStructures =
      nullptr;
  PFN_vkCmdBuildAccelerationStructuresIndirectKHR
      cmdBuildAccelerationStructuresIndirect = nullptr;
  PFN_vkBuildAccelerationStructuresKHR buildAccelerationStructures = nullptr;
  PFN_vkCopyAccelerationStructureKHR copyAccelerationStructure = nullptr;
  PFN_vkCopyAccelerationStructureToMemoryKHR copyAccelerationStructureToMemory =
      nullptr;
  PFN_vkCopyMemoryToAccelerationStructureKHR copyMemoryToAccelerationStructure =
      nullptr;
  PFN_vkWriteAccelerationStructuresPropertiesKHR
      writeAccelerationStructuresProperties = nullptr;
  PFN_vkCmdCopyAccelerationStructureKHR cmdCopyAccelerationStructure = nullptr;
  PFN_vkCmdCopyAccelerationStructureToMemoryKHR
      cmdCopyAccelerationStructureToMemory = nullptr;
  PFN_vkCmdCopyMemoryToAccelerationStructureKHR
      cmdCopyMemoryToAccelerationStructure = nullptr;
  PFN_vkGetAccelerationStructureDeviceAddressKHR
      getAccelerationStructureDeviceAddress = nullptr;
  PFN_vkCmdWriteAccelerationStructuresPropertiesKHR
      cmdWriteAccelerationStructuresProperties = nullptr;
  PFN_vkGetDeviceAccelerationStructureCompatibilityKHR
      getDeviceAccelerationStructureCompatibility = nullptr;
  PFN_vkGetAccelerationStructureBuildSizesKHR
      getAccelerationStructureBuildSizes = nullptr;

  // Ray-tracing pipelines
  PFN_vkCreateRayTracingPipelinesKHR createRayTracingPipelines = nullptr;
  PFN_vkGetRayTracingShaderGroupHandlesKHR getRayTracingShaderGroupHandles =
      nullptr;
  PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR
      getRayTracingCaptureReplayShaderGroupHandles = nullptr;
  PFN_vkCmdTraceRaysKHR cmdTraceRays = nullptr;
  PFN_vkCmdTraceRaysIndirectKHR cmdTraceRaysIndirect = nullptr;
  PFN_vkGetRayTracingShaderGroupStackSizeKHR getRayTracingShaderGroupStackSize =
      nullptr;
  PFN_vkCmdSetRayTracingPipelineStackSizeKHR cmdSetRayTracingPipelineStackSize =
      nullptr;
  // Ray-tracing maintenance 1
  PFN_vkCmdTraceRaysIndirect2KHR cmdTraceRaysIndirect2 = nullptr;

  // Calibrated timestamps
  PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR
      getPhysicalDeviceCalibrateableTimeDomains = nullptr;
  PFN_vkGetCalibratedTimestampsKHR getCalibratedTimestamps = nullptr;

  // Descriptor heaps
  PFN_vkCmdBindResourceHeapEXT cmdBindResourceHeap = nullptr;
  PFN_vkCmdBindSamplerHeapEXT cmdBindSamplerHeap = nullptr;
  PFN_vkCmdPushDataEXT cmdPushData = nullptr;
  PFN_vkGetImageOpaqueCaptureDataEXT getImageOpaqueCaptureData = nullptr;
  PFN_vkGetPhysicalDeviceDescriptorSizeEXT getPhysicalDeviceDescriptorSize =
      nullptr;
  PFN_vkWriteResourceDescriptorsEXT writeResourceDescriptors = nullptr;
  PFN_vkWriteSamplerDescriptorsEXT writeSamplerDescriptors = nullptr;

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
};

[[nodiscard]]
inline PNF_Functions
load_pnf_functions(VkInstance instance, VkDevice device,
                   const ContextProperties &props) noexcept {
  assert(device != VK_NULL_HANDLE);

  PNF_Functions functions{};

  if (props.deferredHostOperations) {
    functions.createDeferredOperation =
        reinterpret_cast<PFN_vkCreateDeferredOperationKHR>(
            vkGetDeviceProcAddr(device, "vkCreateDeferredOperationKHR"));
    functions.destroyDeferredOperation =
        reinterpret_cast<PFN_vkDestroyDeferredOperationKHR>(
            vkGetDeviceProcAddr(device, "vkDestroyDeferredOperationKHR"));
    functions.getDeferredOperationMaxConcurrency =
        reinterpret_cast<PFN_vkGetDeferredOperationMaxConcurrencyKHR>(
            vkGetDeviceProcAddr(device,
                                "vkGetDeferredOperationMaxConcurrencyKHR"));
    functions.getDeferredOperationResult =
        reinterpret_cast<PFN_vkGetDeferredOperationResultKHR>(
            vkGetDeviceProcAddr(device, "vkGetDeferredOperationResultKHR"));
    functions.deferredOperationJoin =
        reinterpret_cast<PFN_vkDeferredOperationJoinKHR>(
            vkGetDeviceProcAddr(device, "vkDeferredOperationJoinKHR"));
  }

  if (props.accelerationStructure) {
    functions.createAccelerationStructure =
        reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
    functions.destroyAccelerationStructure =
        reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
    functions.cmdBuildAccelerationStructures =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
    functions.cmdBuildAccelerationStructuresIndirect =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresIndirectKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCmdBuildAccelerationStructuresIndirectKHR"));
    functions.buildAccelerationStructures =
        reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
    functions.copyAccelerationStructure =
        reinterpret_cast<PFN_vkCopyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureKHR"));
    functions.copyAccelerationStructureToMemory =
        reinterpret_cast<PFN_vkCopyAccelerationStructureToMemoryKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCopyAccelerationStructureToMemoryKHR"));
    functions.copyMemoryToAccelerationStructure =
        reinterpret_cast<PFN_vkCopyMemoryToAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCopyMemoryToAccelerationStructureKHR"));
    functions.writeAccelerationStructuresProperties =
        reinterpret_cast<PFN_vkWriteAccelerationStructuresPropertiesKHR>(
            vkGetDeviceProcAddr(device,
                                "vkWriteAccelerationStructuresPropertiesKHR"));
    functions.cmdCopyAccelerationStructure =
        reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR"));
    functions.cmdCopyAccelerationStructureToMemory =
        reinterpret_cast<PFN_vkCmdCopyAccelerationStructureToMemoryKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCmdCopyAccelerationStructureToMemoryKHR"));
    functions.cmdCopyMemoryToAccelerationStructure =
        reinterpret_cast<PFN_vkCmdCopyMemoryToAccelerationStructureKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCmdCopyMemoryToAccelerationStructureKHR"));
    functions.getAccelerationStructureDeviceAddress =
        reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
            vkGetDeviceProcAddr(device,
                                "vkGetAccelerationStructureDeviceAddressKHR"));
    functions.cmdWriteAccelerationStructuresProperties =
        reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(
            vkGetDeviceProcAddr(
                device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
    functions.getDeviceAccelerationStructureCompatibility =
        reinterpret_cast<PFN_vkGetDeviceAccelerationStructureCompatibilityKHR>(
            vkGetDeviceProcAddr(
                device, "vkGetDeviceAccelerationStructureCompatibilityKHR"));
    functions.getAccelerationStructureBuildSizes =
        reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
            vkGetDeviceProcAddr(device,
                                "vkGetAccelerationStructureBuildSizesKHR"));
  }

  if (props.raytracingPipeline) {
    functions.createRayTracingPipelines =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
    functions.getRayTracingShaderGroupHandles =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
            vkGetDeviceProcAddr(device,
                                "vkGetRayTracingShaderGroupHandlesKHR"));
    functions.getRayTracingCaptureReplayShaderGroupHandles =
        reinterpret_cast<PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR>(
            vkGetDeviceProcAddr(
                device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR"));
    functions.cmdTraceRays = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
    functions.cmdTraceRaysIndirect =
        reinterpret_cast<PFN_vkCmdTraceRaysIndirectKHR>(
            vkGetDeviceProcAddr(device, "vkCmdTraceRaysIndirectKHR"));
    functions.getRayTracingShaderGroupStackSize =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupStackSizeKHR>(
            vkGetDeviceProcAddr(device,
                                "vkGetRayTracingShaderGroupStackSizeKHR"));
    functions.cmdSetRayTracingPipelineStackSize =
        reinterpret_cast<PFN_vkCmdSetRayTracingPipelineStackSizeKHR>(
            vkGetDeviceProcAddr(device,
                                "vkCmdSetRayTracingPipelineStackSizeKHR"));
  }

  if (props.raytracingPipelineMain1) {
    functions.cmdTraceRaysIndirect2 =
        reinterpret_cast<PFN_vkCmdTraceRaysIndirect2KHR>(
            vkGetDeviceProcAddr(device, "vkCmdTraceRaysIndirect2KHR"));
  }

  if (props.calibratedTimestamps) {
    functions.getPhysicalDeviceCalibrateableTimeDomains =
        reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR>(
            vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR"));

    functions.getCalibratedTimestamps =
        reinterpret_cast<PFN_vkGetCalibratedTimestampsKHR>(
            vkGetDeviceProcAddr(device, "vkGetCalibratedTimestampsKHR"));
  }

  if (props.descriptorHeap) {
    functions.cmdBindResourceHeap =
        reinterpret_cast<PFN_vkCmdBindResourceHeapEXT>(
            vkGetDeviceProcAddr(device, "vkCmdBindResourceHeapEXT"));
    functions.cmdBindSamplerHeap =
        reinterpret_cast<PFN_vkCmdBindSamplerHeapEXT>(
            vkGetDeviceProcAddr(device, "vkCmdBindSamplerHeapEXT"));
    functions.cmdPushData = reinterpret_cast<PFN_vkCmdPushDataEXT>(
        vkGetDeviceProcAddr(device, "vkCmdPushDataEXT"));
    functions.getImageOpaqueCaptureData =
        reinterpret_cast<PFN_vkGetImageOpaqueCaptureDataEXT>(
            vkGetDeviceProcAddr(device, "vkGetImageOpaqueCaptureDataEXT"));
    functions.getPhysicalDeviceDescriptorSize =
        reinterpret_cast<PFN_vkGetPhysicalDeviceDescriptorSizeEXT>(
            vkGetInstanceProcAddr(instance,
                                  "vkGetPhysicalDeviceDescriptorSizeEXT"));
    functions.writeResourceDescriptors =
        reinterpret_cast<PFN_vkWriteResourceDescriptorsEXT>(
            vkGetDeviceProcAddr(device, "vkWriteResourceDescriptorsEXT"));
    functions.writeSamplerDescriptors =
        reinterpret_cast<PFN_vkWriteSamplerDescriptorsEXT>(
            vkGetDeviceProcAddr(device, "vkWriteSamplerDescriptorsEXT"));
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
// Deferred host operations
// -----------------------------------------------------------------------------

inline VkResult vk_create_deferred_operation(
    const PNF_Functions *functions, VkDevice device,
    const VkAllocationCallbacks *allocator,
    VkDeferredOperationKHR *deferredOperation) noexcept {
  assert(functions != nullptr);
  assert(functions->createDeferredOperation != nullptr);
  return functions->createDeferredOperation(device, allocator,
                                            deferredOperation);
}

inline void vk_destroy_deferred_operation(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkAllocationCallbacks *allocator = nullptr) noexcept {
  assert(functions != nullptr);
  assert(functions->destroyDeferredOperation != nullptr);
  functions->destroyDeferredOperation(device, deferredOperation, allocator);
}

inline uint32_t vk_get_deferred_operation_max_concurrency(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation) noexcept {
  assert(functions != nullptr);
  assert(functions->getDeferredOperationMaxConcurrency != nullptr);
  return functions->getDeferredOperationMaxConcurrency(device,
                                                       deferredOperation);
}

inline VkResult vk_get_deferred_operation_result(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation) noexcept {
  assert(functions != nullptr);
  assert(functions->getDeferredOperationResult != nullptr);
  return functions->getDeferredOperationResult(device, deferredOperation);
}

inline VkResult
vk_deferred_operation_join(const PNF_Functions *functions, VkDevice device,
                           VkDeferredOperationKHR deferredOperation) noexcept {
  assert(functions != nullptr);
  assert(functions->deferredOperationJoin != nullptr);
  return functions->deferredOperationJoin(device, deferredOperation);
}

// -----------------------------------------------------------------------------
// Acceleration structures
// -----------------------------------------------------------------------------

inline VkResult vk_create_acceleration_structure(
    const PNF_Functions *functions, VkDevice device,
    const VkAccelerationStructureCreateInfoKHR *createInfo,
    const VkAllocationCallbacks *allocator,
    VkAccelerationStructureKHR *accelerationStructure) noexcept {
  assert(functions != nullptr);
  assert(functions->createAccelerationStructure != nullptr);
  return functions->createAccelerationStructure(device, createInfo, allocator,
                                                accelerationStructure);
}

inline void vk_destroy_acceleration_structure(
    const PNF_Functions *functions, VkDevice device,
    VkAccelerationStructureKHR accelerationStructure,
    const VkAllocationCallbacks *allocator = nullptr) noexcept {
  assert(functions != nullptr);
  assert(functions->destroyAccelerationStructure != nullptr);
  functions->destroyAccelerationStructure(device, accelerationStructure,
                                          allocator);
}

inline void vk_cmd_build_acceleration_structures(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *infos,
    const VkAccelerationStructureBuildRangeInfoKHR *const
        *buildRangeInfos) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdBuildAccelerationStructures != nullptr);
  functions->cmdBuildAccelerationStructures(commandBuffer, infoCount, infos,
                                            buildRangeInfos);
}

inline void vk_cmd_build_acceleration_structures_indirect(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *infos,
    const VkDeviceAddress *indirectDeviceAddresses,
    const uint32_t *indirectStrides,
    const uint32_t *const *maxPrimitiveCounts) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdBuildAccelerationStructuresIndirect != nullptr);
  functions->cmdBuildAccelerationStructuresIndirect(
      commandBuffer, infoCount, infos, indirectDeviceAddresses, indirectStrides,
      maxPrimitiveCounts);
}

inline VkResult vk_build_acceleration_structures(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *infos,
    const VkAccelerationStructureBuildRangeInfoKHR *const
        *buildRangeInfos) noexcept {
  assert(functions != nullptr);
  assert(functions->buildAccelerationStructures != nullptr);
  return functions->buildAccelerationStructures(
      device, deferredOperation, infoCount, infos, buildRangeInfos);
}

inline VkResult vk_copy_acceleration_structure(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyAccelerationStructureInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->copyAccelerationStructure != nullptr);
  return functions->copyAccelerationStructure(device, deferredOperation, info);
}

inline VkResult vk_copy_acceleration_structure_to_memory(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->copyAccelerationStructureToMemory != nullptr);
  return functions->copyAccelerationStructureToMemory(device, deferredOperation,
                                                      info);
}

inline VkResult vk_copy_memory_to_acceleration_structure(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->copyMemoryToAccelerationStructure != nullptr);
  return functions->copyMemoryToAccelerationStructure(device, deferredOperation,
                                                      info);
}

inline VkResult vk_write_acceleration_structures_properties(
    const PNF_Functions *functions, VkDevice device,
    uint32_t accelerationStructureCount,
    const VkAccelerationStructureKHR *accelerationStructures,
    VkQueryType queryType, size_t dataSize, void *data,
    size_t stride) noexcept {
  assert(functions != nullptr);
  assert(functions->writeAccelerationStructuresProperties != nullptr);
  return functions->writeAccelerationStructuresProperties(
      device, accelerationStructureCount, accelerationStructures, queryType,
      dataSize, data, stride);
}

inline void vk_cmd_copy_acceleration_structure(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkCopyAccelerationStructureInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdCopyAccelerationStructure != nullptr);
  functions->cmdCopyAccelerationStructure(commandBuffer, info);
}

inline void vk_cmd_copy_acceleration_structure_to_memory(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdCopyAccelerationStructureToMemory != nullptr);
  functions->cmdCopyAccelerationStructureToMemory(commandBuffer, info);
}

inline void vk_cmd_copy_memory_to_acceleration_structure(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdCopyMemoryToAccelerationStructure != nullptr);
  functions->cmdCopyMemoryToAccelerationStructure(commandBuffer, info);
}

inline VkDeviceAddress vk_get_acceleration_structure_device_address(
    const PNF_Functions *functions, VkDevice device,
    const VkAccelerationStructureDeviceAddressInfoKHR *info) noexcept {
  assert(functions != nullptr);
  assert(functions->getAccelerationStructureDeviceAddress != nullptr);
  return functions->getAccelerationStructureDeviceAddress(device, info);
}

inline void vk_cmd_write_acceleration_structures_properties(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    uint32_t accelerationStructureCount,
    const VkAccelerationStructureKHR *accelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool,
    uint32_t firstQuery) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdWriteAccelerationStructuresProperties != nullptr);
  functions->cmdWriteAccelerationStructuresProperties(
      commandBuffer, accelerationStructureCount, accelerationStructures,
      queryType, queryPool, firstQuery);
}

inline void vk_get_device_acceleration_structure_compatibility(
    const PNF_Functions *functions, VkDevice device,
    const VkAccelerationStructureVersionInfoKHR *versionInfo,
    VkAccelerationStructureCompatibilityKHR *compatibility) noexcept {
  assert(functions != nullptr);
  assert(functions->getDeviceAccelerationStructureCompatibility != nullptr);
  functions->getDeviceAccelerationStructureCompatibility(device, versionInfo,
                                                         compatibility);
}

inline void vk_get_acceleration_structure_build_sizes(
    const PNF_Functions *functions, VkDevice device,
    VkAccelerationStructureBuildTypeKHR buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR *buildInfo,
    const uint32_t *maxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR *sizeInfo) noexcept {
  assert(functions != nullptr);
  assert(functions->getAccelerationStructureBuildSizes != nullptr);
  functions->getAccelerationStructureBuildSizes(device, buildType, buildInfo,
                                                maxPrimitiveCounts, sizeInfo);
}

// -----------------------------------------------------------------------------
// Ray-tracing pipelines
// -----------------------------------------------------------------------------

inline VkResult vk_create_raytracing_pipelines(
    const PNF_Functions *functions, VkDevice device,
    VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR *createInfos,
    const VkAllocationCallbacks *allocator, VkPipeline *pipelines) noexcept {
  assert(functions != nullptr);
  assert(functions->createRayTracingPipelines != nullptr);
  return functions->createRayTracingPipelines(
      device, deferredOperation, pipelineCache, createInfoCount, createInfos,
      allocator, pipelines);
}

inline VkResult
vk_get_raytracing_shader_group_handles(const PNF_Functions *functions,
                                       VkDevice device, VkPipeline pipeline,
                                       uint32_t firstGroup, uint32_t groupCount,
                                       size_t dataSize, void *data) noexcept {
  assert(functions != nullptr);
  assert(functions->getRayTracingShaderGroupHandles != nullptr);
  return functions->getRayTracingShaderGroupHandles(
      device, pipeline, firstGroup, groupCount, dataSize, data);
}

inline VkResult vk_get_raytracing_capture_replay_shader_group_handles(
    const PNF_Functions *functions, VkDevice device, VkPipeline pipeline,
    uint32_t firstGroup, uint32_t groupCount, size_t dataSize,
    void *data) noexcept {
  assert(functions != nullptr);
  assert(functions->getRayTracingCaptureReplayShaderGroupHandles != nullptr);
  return functions->getRayTracingCaptureReplayShaderGroupHandles(
      device, pipeline, firstGroup, groupCount, dataSize, data);
}

inline void vk_cmd_trace_rays(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkStridedDeviceAddressRegionKHR *raygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *missShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *hitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *callableShaderBindingTable,
    uint32_t width, uint32_t height, uint32_t depth) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdTraceRays != nullptr);
  functions->cmdTraceRays(commandBuffer, raygenShaderBindingTable,
                          missShaderBindingTable, hitShaderBindingTable,
                          callableShaderBindingTable, width, height, depth);
}

inline void vk_cmd_trace_rays_indirect(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkStridedDeviceAddressRegionKHR *raygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *missShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *hitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *callableShaderBindingTable,
    VkDeviceAddress indirectDeviceAddress) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdTraceRaysIndirect != nullptr);
  functions->cmdTraceRaysIndirect(
      commandBuffer, raygenShaderBindingTable, missShaderBindingTable,
      hitShaderBindingTable, callableShaderBindingTable, indirectDeviceAddress);
}

inline VkDeviceSize vk_get_raytracing_shader_group_stack_size(
    const PNF_Functions *functions, VkDevice device, VkPipeline pipeline,
    uint32_t group, VkShaderGroupShaderKHR groupShader) noexcept {
  assert(functions != nullptr);
  assert(functions->getRayTracingShaderGroupStackSize != nullptr);
  return functions->getRayTracingShaderGroupStackSize(device, pipeline, group,
                                                      groupShader);
}

inline void
vk_cmd_set_raytracing_pipeline_stack_size(const PNF_Functions *functions,
                                          VkCommandBuffer commandBuffer,
                                          uint32_t pipelineStackSize) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdSetRayTracingPipelineStackSize != nullptr);
  functions->cmdSetRayTracingPipelineStackSize(commandBuffer,
                                               pipelineStackSize);
}

// -----------------------------------------------------------------------------
// raytracing maintenance 1
// -----------------------------------------------------------------------------

inline void
vk_cmd_tracy_rays_indirect_2(const PNF_Functions *functions,
                             VkCommandBuffer cmd,
                             VkDeviceAddress indirectDeviceAddress) {
  assert(functions);
  assert(functions->cmdTraceRaysIndirect2);
  functions->cmdTraceRaysIndirect2(cmd, indirectDeviceAddress);
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

// -----------------------------------------------------------------------------
// Descriptor heaps
// -----------------------------------------------------------------------------

inline void vk_cmd_bind_resource_heap(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkBindHeapInfoEXT *bindInfo) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdBindResourceHeap != nullptr);
  functions->cmdBindResourceHeap(commandBuffer, bindInfo);
}

inline void vk_cmd_bind_sampler_heap(
    const PNF_Functions *functions, VkCommandBuffer commandBuffer,
    const VkBindHeapInfoEXT *bindInfo) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdBindSamplerHeap != nullptr);
  functions->cmdBindSamplerHeap(commandBuffer, bindInfo);
}

inline void vk_cmd_push_data(const PNF_Functions *functions,
                             VkCommandBuffer commandBuffer,
                             const VkPushDataInfoEXT *pushDataInfo) noexcept {
  assert(functions != nullptr);
  assert(functions->cmdPushData != nullptr);
  functions->cmdPushData(commandBuffer, pushDataInfo);
}

inline VkResult vk_get_image_opaque_capture_data(
    const PNF_Functions *functions, VkDevice device, uint32_t imageCount,
    const VkImage *images, VkHostAddressRangeEXT *data) noexcept {
  assert(functions != nullptr);
  assert(functions->getImageOpaqueCaptureData != nullptr);
  return functions->getImageOpaqueCaptureData(device, imageCount, images,
                                              data);
}

inline VkDeviceSize vk_get_physical_device_descriptor_size(
    const PNF_Functions *functions, VkPhysicalDevice physicalDevice,
    VkDescriptorType descriptorType) noexcept {
  assert(functions != nullptr);
  assert(functions->getPhysicalDeviceDescriptorSize != nullptr);
  return functions->getPhysicalDeviceDescriptorSize(physicalDevice,
                                                    descriptorType);
}

inline VkResult vk_write_resource_descriptors(
    const PNF_Functions *functions, VkDevice device, uint32_t resourceCount,
    const VkResourceDescriptorInfoEXT *resources,
    const VkHostAddressRangeEXT *descriptors) noexcept {
  assert(functions != nullptr);
  assert(functions->writeResourceDescriptors != nullptr);
  return functions->writeResourceDescriptors(device, resourceCount, resources,
                                             descriptors);
}

inline VkResult vk_write_sampler_descriptors(
    const PNF_Functions *functions, VkDevice device, uint32_t samplerCount,
    const VkSamplerCreateInfo *samplers,
    const VkHostAddressRangeEXT *descriptors) noexcept {
  assert(functions != nullptr);
  assert(functions->writeSamplerDescriptors != nullptr);
  return functions->writeSamplerDescriptors(device, samplerCount, samplers,
                                            descriptors);
}

} // namespace strobe::rhi::vulkan
