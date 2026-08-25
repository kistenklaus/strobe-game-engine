#include "strobe/rhi/vulkan/device_info/device_properties.hpp"

namespace strobe::rhi::vulkan {

template <typename T, std::size_t N>
[[nodiscard]]
static constexpr std::array<T, N> to_array(const T (&source)[N]) {
  std::array<T, N> result{};
  std::ranges::copy(source, result.begin());
  return result;
}

DeviceProperties details::query_device_properties(
    VkInstance instance, VkPhysicalDevice physicalDevice,
    const DeviceFeatures *features,
    const strobe::rhi::allocator_ref alloc) noexcept {

  void *pNext = nullptr;

  VkPhysicalDeviceAccelerationStructurePropertiesKHR asProps{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
      .pNext = nullptr,
      .maxGeometryCount = 0,
      .maxInstanceCount = 0,
      .maxPrimitiveCount = 0,
      .maxPerStageDescriptorAccelerationStructures = 0,
      .maxPerStageDescriptorUpdateAfterBindAccelerationStructures = 0,
      .maxDescriptorSetAccelerationStructures = 0,
      .maxDescriptorSetUpdateAfterBindAccelerationStructures = 0,
      .minAccelerationStructureScratchOffsetAlignment = 0,
  };
  if (features->accelerationStructure) {
    pNext = &asProps;
  }

  VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
      .pNext = pNext,
      .shaderGroupHandleSize = 0,
      .maxRayRecursionDepth = 0,
      .maxShaderGroupStride = 0,
      .shaderGroupBaseAlignment = 0,
      .shaderGroupHandleCaptureReplaySize = 0,
      .maxRayDispatchInvocationCount = 0,
      .shaderGroupHandleAlignment = 0,
      .maxRayHitAttributeSize = 0,
  };
  if (features->rayTracingPipeline) {
    pNext = &rtProps;
  }

  VkPhysicalDeviceProperties2 props{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
      .pNext = pNext,
      .properties = {},
  };
  vkGetPhysicalDeviceProperties2(physicalDevice, &props);

  SmallVector<VkTimeDomainKHR, 4> timeDomains;
  if (features->calibratedTimestamps) {
    const auto pfn_query_time_domains =
        reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR>(
            vkGetInstanceProcAddr(
                instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR"));
    assert(pfn_query_time_domains != nullptr);
    VkResult result;
    do {
      uint32_t count = 0;
      result = pfn_query_time_domains(physicalDevice, &count, nullptr);
      assert(result == VK_SUCCESS);
      timeDomains.resize(count);
      if (count == 0) {
        break;
      }
      result =
          pfn_query_time_domains(physicalDevice, &count, timeDomains.data());
      if (result == VK_SUCCESS) {
        timeDomains.resize(count);
      }
    } while (result == VK_INCOMPLETE);
    assert(result == VK_SUCCESS);
  }

  VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptorHeapProperties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT,
      .pNext = nullptr,
      .samplerHeapAlignment = 0,
      .resourceHeapAlignment = 0,
      .maxSamplerHeapSize = 0,
      .maxResourceHeapSize = 0,
      .minSamplerHeapReservedRange = 0,
      .minSamplerHeapReservedRangeWithEmbedded = 0,
      .minResourceHeapReservedRange = 0,
      .samplerDescriptorSize = 0,
      .imageDescriptorSize = 0,
      .bufferDescriptorSize = 0,
      .samplerDescriptorAlignment = 0,
      .imageDescriptorAlignment = 0,
      .bufferDescriptorAlignment = 0,
      .maxPushDataSize = 0,
      .imageCaptureReplayOpaqueDataSize = 0,
      .maxDescriptorHeapEmbeddedSamplers = 0,
      .samplerYcbcrConversionCount = 0,
      .sparseDescriptorHeaps = VK_FALSE,
      .protectedDescriptorHeaps = VK_FALSE,
  };
  if (features->descriptorHeap) {
    descriptorHeapProperties.pNext = pNext;
    pNext = &descriptorHeapProperties;
  }

  return DeviceProperties{
      .apiVersion = props.properties.apiVersion,
      .deviceName = {props.properties.deviceName, alloc},
      .deviceID = props.properties.deviceID,
      .deviceType = props.properties.deviceType,
      .driverVersion = props.properties.driverVersion,
      .pipelineCacheUUID = to_array(props.properties.pipelineCacheUUID),
      .limits =
          {
              .maxImageDimension1D =
                  props.properties.limits.maxImageDimension1D,
              .maxImageDimension2D =
                  props.properties.limits.maxImageDimension2D,
              .maxImageDimension3D =
                  props.properties.limits.maxImageDimension3D,
              .maxImageDimensionCube =
                  props.properties.limits.maxImageDimensionCube,
              .maxImageArrayLayers =
                  props.properties.limits.maxImageArrayLayers,
              .maxTexelBufferElements =
                  props.properties.limits.maxTexelBufferElements,
              .maxUniformBufferRange =
                  props.properties.limits.maxUniformBufferRange,
              .maxStorageBufferRange =
                  props.properties.limits.maxStorageBufferRange,
              .maxPushConstantsSize =
                  props.properties.limits.maxPushConstantsSize,
              .maxMemoryAllocationCount =
                  props.properties.limits.maxMemoryAllocationCount,
              .maxSamplerAllocationCount =
                  props.properties.limits.maxSamplerAllocationCount,
              .bufferImageGranularity =
                  props.properties.limits.bufferImageGranularity,
              .sparseAddressSpaceSize =
                  props.properties.limits.sparseAddressSpaceSize,
              .maxBoundDescriptorSets =
                  props.properties.limits.maxBoundDescriptorSets,
              .maxPerStageDescriptorSamplers =
                  props.properties.limits.maxPerStageDescriptorSamplers,
              .maxPerStageDescriptorUniformBuffers =
                  props.properties.limits.maxPerStageDescriptorUniformBuffers,
              .maxPerStageDescriptorStorageBuffers =
                  props.properties.limits.maxPerStageDescriptorStorageBuffers,
              .maxPerStageDescriptorSampledImages =
                  props.properties.limits.maxPerStageDescriptorSampledImages,
              .maxPerStageDescriptorStorageImages =
                  props.properties.limits.maxPerStageDescriptorStorageImages,
              .maxPerStageDescriptorInputAttachments =
                  props.properties.limits.maxPerStageDescriptorInputAttachments,
              .maxPerStageResources =
                  props.properties.limits.maxPerStageResources,
              .maxDescriptorSetSamplers =
                  props.properties.limits.maxDescriptorSetSamplers,
              .maxDescriptorSetUniformBuffers =
                  props.properties.limits.maxDescriptorSetUniformBuffers,
              .maxDescriptorSetUniformBuffersDynamic =
                  props.properties.limits.maxDescriptorSetUniformBuffersDynamic,
              .maxDescriptorSetStorageBuffers =
                  props.properties.limits.maxDescriptorSetStorageBuffers,
              .maxDescriptorSetStorageBuffersDynamic =
                  props.properties.limits.maxDescriptorSetStorageBuffersDynamic,
              .maxDescriptorSetSampledImages =
                  props.properties.limits.maxDescriptorSetSampledImages,
              .maxDescriptorSetStorageImages =
                  props.properties.limits.maxDescriptorSetStorageImages,
              .maxDescriptorSetInputAttachments =
                  props.properties.limits.maxDescriptorSetInputAttachments,
              .maxVertexInputAttributes =
                  props.properties.limits.maxVertexInputAttributes,
              .maxVertexInputBindings =
                  props.properties.limits.maxVertexInputBindings,
              .maxVertexInputAttributeOffset =
                  props.properties.limits.maxVertexInputAttributeOffset,
              .maxVertexInputBindingStride =
                  props.properties.limits.maxVertexInputBindingStride,
              .maxVertexOutputComponents =
                  props.properties.limits.maxVertexOutputComponents,
              .maxTessellationGenerationLevel =
                  props.properties.limits.maxTessellationGenerationLevel,
              .maxTessellationPatchSize =
                  props.properties.limits.maxTessellationPatchSize,
              .maxTessellationControlPerVertexInputComponents =
                  props.properties.limits
                      .maxTessellationControlPerVertexInputComponents,
              .maxTessellationControlPerVertexOutputComponents =
                  props.properties.limits
                      .maxTessellationControlPerVertexOutputComponents,
              .maxTessellationControlPerPatchOutputComponents =
                  props.properties.limits
                      .maxTessellationControlPerPatchOutputComponents,
              .maxTessellationControlTotalOutputComponents =
                  props.properties.limits
                      .maxTessellationControlTotalOutputComponents,
              .maxTessellationEvaluationInputComponents =
                  props.properties.limits
                      .maxTessellationEvaluationInputComponents,
              .maxTessellationEvaluationOutputComponents =
                  props.properties.limits
                      .maxTessellationEvaluationOutputComponents,
              .maxGeometryShaderInvocations =
                  props.properties.limits.maxGeometryShaderInvocations,
              .maxGeometryInputComponents =
                  props.properties.limits.maxGeometryInputComponents,
              .maxGeometryOutputComponents =
                  props.properties.limits.maxGeometryOutputComponents,
              .maxGeometryOutputVertices =
                  props.properties.limits.maxGeometryOutputVertices,
              .maxGeometryTotalOutputComponents =
                  props.properties.limits.maxGeometryTotalOutputComponents,
              .maxFragmentInputComponents =
                  props.properties.limits.maxFragmentInputComponents,
              .maxFragmentOutputAttachments =
                  props.properties.limits.maxFragmentOutputAttachments,
              .maxFragmentDualSrcAttachments =
                  props.properties.limits.maxFragmentDualSrcAttachments,
              .maxFragmentCombinedOutputResources =
                  props.properties.limits.maxFragmentCombinedOutputResources,
              .maxComputeSharedMemorySize =
                  props.properties.limits.maxComputeSharedMemorySize,
              .maxComputeWorkGroupCount =
                  {props.properties.limits.maxComputeWorkGroupCount[0],
                   props.properties.limits.maxComputeWorkGroupCount[1],
                   props.properties.limits.maxComputeWorkGroupCount[2]},
              .maxComputeWorkGroupInvocations =
                  props.properties.limits.maxComputeWorkGroupInvocations,
              .maxComputeWorkGroupSize =
                  {
                      props.properties.limits.maxComputeWorkGroupSize[0],
                      props.properties.limits.maxComputeWorkGroupSize[1],
                      props.properties.limits.maxComputeWorkGroupSize[2],
                  },
              .subPixelPrecisionBits =
                  props.properties.limits.subPixelPrecisionBits,
              .subTexelPrecisionBits =
                  props.properties.limits.subTexelPrecisionBits,
              .mipmapPrecisionBits =
                  props.properties.limits.mipmapPrecisionBits,
              .maxDrawIndexedIndexValue =
                  props.properties.limits.maxDrawIndexedIndexValue,
              .maxDrawIndirectCount =
                  props.properties.limits.maxDrawIndirectCount,
              .maxSamplerLodBias = props.properties.limits.maxSamplerLodBias,
              .maxSamplerAnisotropy =
                  props.properties.limits.maxSamplerAnisotropy,
              .maxViewports = props.properties.limits.maxViewports,
              .maxViewportDimensions =
                  {
                      props.properties.limits.maxViewportDimensions[0],
                      props.properties.limits.maxViewportDimensions[1],
                  },
              .viewportBoundsRange =
                  {
                      props.properties.limits.viewportBoundsRange[0],
                      props.properties.limits.viewportBoundsRange[1],
                  },
              .viewportSubPixelBits =
                  props.properties.limits.viewportSubPixelBits,
              .minMemoryMapAlignment =
                  props.properties.limits.minMemoryMapAlignment,
              .minTexelBufferOffsetAlignment =
                  props.properties.limits.minTexelBufferOffsetAlignment,
              .minUniformBufferOffsetAlignment =
                  props.properties.limits.minUniformBufferOffsetAlignment,
              .minStorageBufferOffsetAlignment =
                  props.properties.limits.minStorageBufferOffsetAlignment,
              .minTexelOffset = props.properties.limits.minTexelOffset,
              .maxTexelOffset = props.properties.limits.maxTexelOffset,
              .minTexelGatherOffset =
                  props.properties.limits.minTexelGatherOffset,
              .maxTexelGatherOffset =
                  props.properties.limits.maxTexelGatherOffset,
              .minInterpolationOffset =
                  props.properties.limits.minInterpolationOffset,
              .maxInterpolationOffset =
                  props.properties.limits.maxInterpolationOffset,
              .subPixelInterpolationOffsetBits =
                  props.properties.limits.subPixelInterpolationOffsetBits,
              .maxFramebufferWidth =
                  props.properties.limits.maxFramebufferWidth,
              .maxFramebufferHeight =
                  props.properties.limits.maxFramebufferHeight,
              .maxFramebufferLayers =
                  props.properties.limits.maxFramebufferLayers,
              .framebufferColorSampleCounts =
                  props.properties.limits.framebufferColorSampleCounts,
              .framebufferDepthSampleCounts =
                  props.properties.limits.framebufferDepthSampleCounts,
              .framebufferStencilSampleCounts =
                  props.properties.limits.framebufferStencilSampleCounts,
              .framebufferNoAttachmentsSampleCounts =
                  props.properties.limits.framebufferNoAttachmentsSampleCounts,
              .maxColorAttachments =
                  props.properties.limits.maxColorAttachments,
              .sampledImageColorSampleCounts =
                  props.properties.limits.sampledImageColorSampleCounts,
              .sampledImageIntegerSampleCounts =
                  props.properties.limits.sampledImageIntegerSampleCounts,
              .sampledImageDepthSampleCounts =
                  props.properties.limits.sampledImageDepthSampleCounts,
              .sampledImageStencilSampleCounts =
                  props.properties.limits.sampledImageStencilSampleCounts,
              .storageImageSampleCounts =
                  props.properties.limits.storageImageSampleCounts,
              .maxSampleMaskWords = props.properties.limits.maxSampleMaskWords,
              .timestampComputeAndGraphics =
                  props.properties.limits.timestampComputeAndGraphics,
              .timestampPeriod = props.properties.limits.timestampPeriod,
              .maxClipDistances = props.properties.limits.maxClipDistances,
              .maxCullDistances = props.properties.limits.maxCullDistances,
              .maxCombinedClipAndCullDistances =
                  props.properties.limits.maxCombinedClipAndCullDistances,
              .discreteQueuePriorities =
                  props.properties.limits.discreteQueuePriorities,
              .pointSizeRange =
                  {
                      props.properties.limits.pointSizeRange[0],
                      props.properties.limits.pointSizeRange[1],
                  },
              .lineWidthRange =
                  {
                      props.properties.limits.lineWidthRange[0],
                      props.properties.limits.lineWidthRange[1],
                  },
              .pointSizeGranularity =
                  props.properties.limits.pointSizeGranularity,
              .lineWidthGranularity =
                  props.properties.limits.lineWidthGranularity,
              .strictLines = props.properties.limits.strictLines,
              .standardSampleLocations =
                  props.properties.limits.standardSampleLocations,
              .optimalBufferCopyOffsetAlignment =
                  props.properties.limits.optimalBufferCopyOffsetAlignment,
              .optimalBufferCopyRowPitchAlignment =
                  props.properties.limits.optimalBufferCopyRowPitchAlignment,
              .nonCoherentAtomSize =
                  props.properties.limits.nonCoherentAtomSize,
          },
      .calibratableTimeDomains = timeDomains,
      .raytracingPipeline =
          RaytracingPipelineProperties{
              .shaderGroupHandleSize = rtProps.shaderGroupHandleSize,
              .maxRayRecursionDepth = rtProps.maxRayRecursionDepth,
              .maxShaderGroupStride = rtProps.maxShaderGroupStride,
              .shaderGroupBaseAlignment = rtProps.shaderGroupBaseAlignment,
              .shaderGroupHandleCaptureReplaySize =
                  rtProps.shaderGroupHandleCaptureReplaySize,
              .maxRayDispatchInvocationCount =
                  rtProps.maxRayDispatchInvocationCount,
              .shaderGroupHandleAlignment = rtProps.shaderGroupHandleAlignment,
              .maxRayHitAttributeSize = rtProps.maxRayHitAttributeSize,
          },
      .accelerationStructure =
          AccelerationStructureProperties{
              .maxGeometryCount = asProps.maxGeometryCount,
              .maxInstanceCount = asProps.maxInstanceCount,
              .maxPrimitiveCount = asProps.maxPrimitiveCount,
              .maxPerStageDescriptorAccelerationStructures =
                  asProps.maxPerStageDescriptorAccelerationStructures,
              .maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
                  asProps
                      .maxPerStageDescriptorUpdateAfterBindAccelerationStructures,
              .maxDescriptorSetAccelerationStructures =
                  asProps.maxDescriptorSetAccelerationStructures,
              .maxDescriptorSetUpdateAfterBindAccelerationStructures =
                  asProps.maxDescriptorSetUpdateAfterBindAccelerationStructures,
              .minAccelerationStructureScratchOffsetAlignment =
                  asProps.minAccelerationStructureScratchOffsetAlignment,
          },
      .descriptorHeap = DescriptorHeapProperties{
          .samplerHeapAlignment = descriptorHeapProperties.samplerHeapAlignment,
          .resourceHeapAlignment =
              descriptorHeapProperties.resourceHeapAlignment,
          .maxSamplerHeapSize = descriptorHeapProperties.maxSamplerHeapSize,
          .maxResourceHeapSize = descriptorHeapProperties.maxResourceHeapSize,
          .minSamplerHeapReservedRange =
              descriptorHeapProperties.minSamplerHeapReservedRange,
          .minSamplerHeapReservedRangeWithEmbedded =
              descriptorHeapProperties.minSamplerHeapReservedRangeWithEmbedded,
          .minResourceHeapReservedRange =
              descriptorHeapProperties.minResourceHeapReservedRange,
          .samplerDescriptorSize =
              descriptorHeapProperties.samplerDescriptorSize,
          .imageDescriptorSize = descriptorHeapProperties.imageDescriptorSize,
          .bufferDescriptorSize = descriptorHeapProperties.bufferDescriptorSize,
          .samplerDescriptorAlignment =
              descriptorHeapProperties.samplerDescriptorAlignment,
          .imageDescriptorAlignment =
              descriptorHeapProperties.imageDescriptorAlignment,
          .bufferDescriptorAlignment =
              descriptorHeapProperties.bufferDescriptorAlignment,
          .maxPushDataSize = descriptorHeapProperties.maxPushDataSize,
          .imageCaptureReplayOpaqueDataSize =
              descriptorHeapProperties.imageCaptureReplayOpaqueDataSize,
          .maxDescriptorHeapEmbeddedSamplers =
              descriptorHeapProperties.maxDescriptorHeapEmbeddedSamplers,
          .samplerYcbcrConversionCount =
              descriptorHeapProperties.samplerYcbcrConversionCount,
          .sparseDescriptorHeaps =
              descriptorHeapProperties.sparseDescriptorHeaps,
          .protectedDescriptorHeaps =
              descriptorHeapProperties.protectedDescriptorHeaps,
      }}; // namespace strobe::rhi::vulkan
}
} // namespace strobe::rhi::vulkan
