#include "strobe/rhi/vulkan/device_info/device_features.hpp"

namespace strobe::rhi::vulkan {

DeviceFeatures details::query_device_features(
    VkPhysicalDevice physicalDevice, uint32_t apiVersion,
    span<const DeviceExtension> supportedExtensions) noexcept {

  void *pNext = nullptr;

  VkPhysicalDeviceVulkan11Features vulkan11{};
  vulkan11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  if (apiVersion >= VK_API_VERSION_1_1) {
    vulkan11.pNext = pNext;
    pNext = &vulkan11;
  }

  VkPhysicalDeviceVulkan12Features vulkan12{};
  vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  if (apiVersion >= VK_API_VERSION_1_2) {
    vulkan12.pNext = pNext;
    pNext = &vulkan12;
  }

  VkPhysicalDeviceVulkan13Features vulkan13{};
  vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  if (apiVersion >= VK_API_VERSION_1_3) {
    vulkan13.pNext = pNext;
    pNext = &vulkan13;
  }

  VkPhysicalDeviceVulkan14Features vulkan14{};
  vulkan14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
  if (apiVersion >= VK_API_VERSION_1_4) {
    vulkan14.pNext = pNext;
    pNext = &vulkan14;
  }

  const bool swapchainMain1Ext = details::supports_extension(
      supportedExtensions, VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
  VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchainMaintenance1{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR,
      .pNext = nullptr,
      .swapchainMaintenance1 = VK_FALSE,
  };
  if (swapchainMain1Ext) {
    swapchainMaintenance1.pNext = pNext;
    pNext = &swapchainMaintenance1;
  }

  const bool shaderObjExt = details::supports_extension(
      supportedExtensions, VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

  VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
      .pNext = nullptr,
      .shaderObject = VK_FALSE,
  };
  if (shaderObjExt) {
    shaderObjectFeatures.pNext = pNext;
    pNext = &shaderObjectFeatures;
  }

  const bool calibratedTimestampsExt = details::supports_extension(
      supportedExtensions, VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);

  const bool raytracingPipelineExt = details::supports_extension(
      supportedExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

  VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
      .pNext = nullptr,
      .rayTracingPipeline = VK_FALSE,
      .rayTracingPipelineShaderGroupHandleCaptureReplay = false,
      .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
      .rayTracingPipelineTraceRaysIndirect = VK_FALSE,
      .rayTraversalPrimitiveCulling = VK_FALSE,
  };
  if (raytracingPipelineExt) {
    raytracingPipelineFeatures.pNext = pNext;
    pNext = &raytracingPipelineFeatures;
  }

  const bool raytracingMaintenance1Ext = details::supports_extension(
      supportedExtensions, VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

  VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR raytracingMain1{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR,
      .pNext = nullptr,
      .rayTracingMaintenance1 = VK_FALSE,
      .rayTracingPipelineTraceRaysIndirect2 = VK_FALSE,
  };
  if (raytracingMaintenance1Ext) {
    raytracingMain1.pNext = pNext;
    pNext = &raytracingMain1;
  }

  const bool accelerationStructureExt = details::supports_extension(
      supportedExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
  VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
      .pNext = nullptr,
      .accelerationStructure = VK_FALSE,
      .accelerationStructureCaptureReplay = VK_FALSE,
      .accelerationStructureIndirectBuild = VK_FALSE,
      .accelerationStructureHostCommands = VK_FALSE,
      .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE,
  };
  if (accelerationStructureExt) {
    accelerationStructureFeatures.pNext = pNext;
    pNext = &accelerationStructureFeatures;
  }

  const bool rayQueryExt = details::supports_extension(
      supportedExtensions, VK_KHR_RAY_QUERY_EXTENSION_NAME);

  VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
      .pNext = nullptr,
      .rayQuery = VK_FALSE,
  };
  if (rayQueryExt) {
    rayQueryFeatures.pNext = pNext;
    pNext = &rayQueryFeatures;
  }

  const bool descriptorHeapExt = details::supports_extension(
      supportedExtensions, VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
  VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
      .pNext = nullptr,
      .descriptorHeap = VK_FALSE,
      .descriptorHeapCaptureReplay = VK_FALSE,
  };

  if (descriptorHeapExt) {
    descriptorHeapFeatures.pNext = pNext;
    pNext = &descriptorHeapFeatures;
  }

  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = pNext,
      .features = {},
  };

  const bool deferredHostOperationsExt = details::supports_extension(
      supportedExtensions, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

  {
    ZoneScopedN("vkGetPhysicalDeviceFeatures2");
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
  }

  return DeviceFeatures{
      .robustBufferAccess = features2.features.robustBufferAccess,
      .fullDrawIndexUint32 = features2.features.fullDrawIndexUint32,
      .imageCubeArray = features2.features.imageCubeArray,
      .independentBlend = features2.features.independentBlend,
      .geometryShader = features2.features.geometryShader,
      .tessellationShader = features2.features.tessellationShader,
      .sampleRateShading = features2.features.sampleRateShading,
      .dualSrcBlend = features2.features.dualSrcBlend,
      .logicOp = features2.features.logicOp,
      .multiDrawIndirect = features2.features.multiDrawIndirect,
      .drawIndirectFirstInstance = features2.features.drawIndirectFirstInstance,
      .depthClamp = features2.features.depthClamp,
      .depthBiasClamp = features2.features.depthBiasClamp,
      .fillModeNonSolid = features2.features.fillModeNonSolid,
      .depthBounds = features2.features.depthBounds,
      .wideLines = features2.features.wideLines,
      .largePoints = features2.features.largePoints,
      .alphaToOne = features2.features.alphaToOne,
      .multiViewport = features2.features.multiViewport,
      .samplerAnisotropy = features2.features.samplerAnisotropy,
      .textureCompressionETC2 = features2.features.textureCompressionETC2,
      .textureCompressionASTC_LDR =
          features2.features.textureCompressionASTC_LDR,
      .textureCompressionBC = features2.features.textureCompressionBC,
      .occlusionQueryPrecise = features2.features.occlusionQueryPrecise,
      .pipelineStatisticsQuery = features2.features.pipelineStatisticsQuery,
      .vertexPipelineStoresAndAtomics =
          features2.features.vertexPipelineStoresAndAtomics,
      .fragmentStoresAndAtomics = features2.features.fragmentStoresAndAtomics,
      .shaderTessellationAndGeometryPointSize =
          features2.features.shaderTessellationAndGeometryPointSize,
      .shaderImageGatherExtended = features2.features.shaderImageGatherExtended,
      .shaderStorageImageExtendedFormats =
          features2.features.shaderStorageImageExtendedFormats,
      .shaderStorageImageMultisample =
          features2.features.shaderStorageImageMultisample,
      .shaderStorageImageReadWithoutFormat =
          features2.features.shaderStorageImageReadWithoutFormat,
      .shaderStorageImageWriteWithoutFormat =
          features2.features.shaderStorageImageWriteWithoutFormat,
      .shaderUniformBufferArrayDynamicIndexing =
          features2.features.shaderUniformBufferArrayDynamicIndexing,
      .shaderSampledImageArrayDynamicIndexing =
          features2.features.shaderSampledImageArrayDynamicIndexing,
      .shaderStorageBufferArrayDynamicIndexing =
          features2.features.shaderStorageBufferArrayDynamicIndexing,
      .shaderStorageImageArrayDynamicIndexing =
          features2.features.shaderStorageImageArrayDynamicIndexing,
      .shaderClipDistance = features2.features.shaderClipDistance,
      .shaderCullDistance = features2.features.shaderCullDistance,
      .shaderFloat64 = features2.features.shaderFloat64,
      .shaderInt64 = features2.features.shaderInt64,
      .shaderInt16 = features2.features.shaderInt16,
      .shaderResourceResidency = features2.features.shaderResourceResidency,
      .shaderResourceMinLod = features2.features.shaderResourceMinLod,
      .sparseBinding = features2.features.sparseBinding,
      .sparseResidencyBuffer = features2.features.sparseResidencyBuffer,
      .sparseResidencyImage2D = features2.features.sparseResidencyImage2D,
      .sparseResidencyImage3D = features2.features.sparseResidencyImage3D,
      .sparseResidency2Samples = features2.features.sparseResidency2Samples,
      .sparseResidency4Samples = features2.features.sparseResidency4Samples,
      .sparseResidency8Samples = features2.features.sparseResidency8Samples,
      .sparseResidency16Samples = features2.features.sparseResidency16Samples,
      .sparseResidencyAliased = features2.features.sparseResidencyAliased,
      .variableMultisampleRate = features2.features.variableMultisampleRate,
      .inheritedQueries = features2.features.inheritedQueries,
      .storageBuffer16BitAccess = vulkan11.storageBuffer16BitAccess,
      .uniformAndStorageBuffer16BitAccess =
          vulkan11.uniformAndStorageBuffer16BitAccess,
      .storagePushConstant16 = vulkan11.storagePushConstant16,
      .storageInputOutput16 = vulkan11.storageInputOutput16,
      .multiview = vulkan11.multiview,
      .multiviewGeometryShader = vulkan11.multiviewGeometryShader,
      .multiviewTessellationShader = vulkan11.multiviewTessellationShader,
      .variablePointersStorageBuffer = vulkan11.variablePointersStorageBuffer,
      .variablePointers = vulkan11.variablePointers,
      .protectedMemory = vulkan11.protectedMemory,
      .samplerYcbcrConversion = vulkan11.samplerYcbcrConversion,
      .shaderDrawParameters = vulkan11.shaderDrawParameters,
      .samplerMirrorClampToEdge = vulkan12.samplerMirrorClampToEdge,
      .drawIndirectCount = vulkan12.drawIndirectCount,
      .storageBuffer8BitAccess = vulkan12.storageBuffer8BitAccess,
      .uniformAndStorageBuffer8BitAccess =
          vulkan12.uniformAndStorageBuffer8BitAccess,
      .storagePushConstant8 = vulkan12.storagePushConstant8,
      .shaderBufferInt64Atomics = vulkan12.shaderBufferInt64Atomics,
      .shaderSharedInt64Atomics = vulkan12.shaderSharedInt64Atomics,
      .shaderFloat16 = vulkan12.shaderFloat16,
      .shaderInt8 = vulkan12.shaderInt8,
      .descriptorIndexing = vulkan12.descriptorIndexing,
      .shaderInputAttachmentArrayDynamicIndexing =
          vulkan12.shaderInputAttachmentArrayDynamicIndexing,
      .shaderUniformTexelBufferArrayDynamicIndexing =
          vulkan12.shaderUniformTexelBufferArrayDynamicIndexing,
      .shaderStorageTexelBufferArrayDynamicIndexing =
          vulkan12.shaderStorageTexelBufferArrayDynamicIndexing,
      .shaderUniformBufferArrayNonUniformIndexing =
          vulkan12.shaderUniformBufferArrayNonUniformIndexing,
      .shaderSampledImageArrayNonUniformIndexing =
          vulkan12.shaderSampledImageArrayNonUniformIndexing,
      .shaderStorageBufferArrayNonUniformIndexing =
          vulkan12.shaderStorageBufferArrayNonUniformIndexing,
      .shaderStorageImageArrayNonUniformIndexing =
          vulkan12.shaderStorageImageArrayNonUniformIndexing,
      .shaderInputAttachmentArrayNonUniformIndexing =
          vulkan12.shaderInputAttachmentArrayNonUniformIndexing,
      .shaderUniformTexelBufferArrayNonUniformIndexing =
          vulkan12.shaderUniformTexelBufferArrayNonUniformIndexing,
      .shaderStorageTexelBufferArrayNonUniformIndexing =
          vulkan12.shaderStorageTexelBufferArrayNonUniformIndexing,
      .descriptorBindingUniformBufferUpdateAfterBind =
          vulkan12.descriptorBindingUniformBufferUpdateAfterBind,
      .descriptorBindingSampledImageUpdateAfterBind =
          vulkan12.descriptorBindingSampledImageUpdateAfterBind,
      .descriptorBindingStorageImageUpdateAfterBind =
          vulkan12.descriptorBindingStorageImageUpdateAfterBind,
      .descriptorBindingStorageBufferUpdateAfterBind =
          vulkan12.descriptorBindingStorageBufferUpdateAfterBind,
      .descriptorBindingUniformTexelBufferUpdateAfterBind =
          vulkan12.descriptorBindingUniformTexelBufferUpdateAfterBind,
      .descriptorBindingStorageTexelBufferUpdateAfterBind =
          vulkan12.descriptorBindingStorageTexelBufferUpdateAfterBind,
      .descriptorBindingUpdateUnusedWhilePending =
          vulkan12.descriptorBindingUpdateUnusedWhilePending,
      .descriptorBindingPartiallyBound =
          vulkan12.descriptorBindingPartiallyBound,
      .descriptorBindingVariableDescriptorCount =
          vulkan12.descriptorBindingVariableDescriptorCount,
      .runtimeDescriptorArray = vulkan12.runtimeDescriptorArray,
      .samplerFilterMinmax = vulkan12.samplerFilterMinmax,
      .scalarBlockLayout = vulkan12.scalarBlockLayout,
      .imagelessFramebuffer = vulkan12.imagelessFramebuffer,
      .uniformBufferStandardLayout = vulkan12.uniformBufferStandardLayout,
      .shaderSubgroupExtendedTypes = vulkan12.shaderSubgroupExtendedTypes,
      .separateDepthStencilLayouts = vulkan12.separateDepthStencilLayouts,
      .hostQueryReset = vulkan12.hostQueryReset,
      .timelineSemaphore = vulkan12.timelineSemaphore,
      .bufferDeviceAddress = vulkan12.bufferDeviceAddress,
      .bufferDeviceAddressCaptureReplay =
          vulkan12.bufferDeviceAddressCaptureReplay,
      .bufferDeviceAddressMultiDevice = vulkan12.bufferDeviceAddressMultiDevice,
      .vulkanMemoryModel = vulkan12.vulkanMemoryModel,
      .vulkanMemoryModelDeviceScope = vulkan12.vulkanMemoryModelDeviceScope,
      .vulkanMemoryModelAvailabilityVisibilityChains =
          vulkan12.vulkanMemoryModelAvailabilityVisibilityChains,
      .shaderOutputViewportIndex = vulkan12.shaderOutputViewportIndex,
      .shaderOutputLayer = vulkan12.shaderOutputLayer,
      .subgroupBroadcastDynamicId = vulkan12.subgroupBroadcastDynamicId,
      .robustImageAccess = vulkan13.robustImageAccess,
      .inlineUniformBlock = vulkan13.inlineUniformBlock,
      .descriptorBindingInlineUniformBlockUpdateAfterBind =
          vulkan13.descriptorBindingInlineUniformBlockUpdateAfterBind,
      .pipelineCreationCacheControl = vulkan13.pipelineCreationCacheControl,
      .privateData = vulkan13.privateData,
      .shaderDemoteToHelperInvocation = vulkan13.shaderDemoteToHelperInvocation,
      .shaderTerminateInvocation = vulkan13.shaderTerminateInvocation,
      .subgroupSizeControl = vulkan13.subgroupSizeControl,
      .computeFullSubgroups = vulkan13.computeFullSubgroups,
      .synchronization2 = vulkan13.synchronization2,
      .textureCompressionASTC_HDR = vulkan13.textureCompressionASTC_HDR,
      .shaderZeroInitializeWorkgroupMemory =
          vulkan13.shaderZeroInitializeWorkgroupMemory,
      .dynamicRendering = vulkan13.dynamicRendering,
      .shaderIntegerDotProduct = vulkan13.shaderIntegerDotProduct,
      .maintenance4 = vulkan13.maintenance4,
      .globalPriorityQuery = vulkan14.globalPriorityQuery,
      .shaderSubgroupRotate = vulkan14.shaderSubgroupRotate,
      .shaderSubgroupRotateClustered = vulkan14.shaderSubgroupRotateClustered,
      .shaderFloatControls2 = vulkan14.shaderFloatControls2,
      .shaderExpectAssume = vulkan14.shaderExpectAssume,
      .rectangularLines = vulkan14.rectangularLines,
      .bresenhamLines = vulkan14.bresenhamLines,
      .smoothLines = vulkan14.smoothLines,
      .stippledRectangularLines = vulkan14.stippledRectangularLines,
      .stippledBresenhamLines = vulkan14.stippledBresenhamLines,
      .stippledSmoothLines = vulkan14.stippledSmoothLines,
      .vertexAttributeInstanceRateDivisor =
          vulkan14.vertexAttributeInstanceRateDivisor,
      .vertexAttributeInstanceRateZeroDivisor =
          vulkan14.vertexAttributeInstanceRateZeroDivisor,
      .indexTypeUint8 = vulkan14.indexTypeUint8,
      .dynamicRenderingLocalRead = vulkan14.dynamicRenderingLocalRead,
      .maintenance5 = vulkan14.maintenance5,
      .maintenance6 = vulkan14.maintenance6,
      .pipelineProtectedAccess = vulkan14.pipelineProtectedAccess,
      .pipelineRobustness = vulkan14.pipelineRobustness,
      .hostImageCopy = vulkan14.hostImageCopy,
      .pushDescriptor = vulkan14.pushDescriptor,
      .swapchainMaintenance1 = swapchainMaintenance1.swapchainMaintenance1,
      .shaderObjects = shaderObjectFeatures.shaderObject,
      .calibratedTimestamps = calibratedTimestampsExt,

      .rayTracingPipeline = raytracingPipelineFeatures.rayTracingPipeline,
      .rayTracingPipelineShaderGroupHandleCaptureReplay =
          raytracingPipelineFeatures
              .rayTracingPipelineShaderGroupHandleCaptureReplay,
      .rayTracingPipelineShaderGroupHandleCaptureReplayMixed =
          raytracingPipelineFeatures
              .rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
      .rayTracingPipelineTraceRaysIndirect =
          raytracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect,
      .rayTraversalPrimitiveCulling =
          raytracingPipelineFeatures.rayTraversalPrimitiveCulling,
      .rayTracingMaintenance1 = raytracingMain1.rayTracingMaintenance1,
      .rayTracingPipelineTraceRaysIndirect2 =
          raytracingMain1.rayTracingPipelineTraceRaysIndirect2,
      .rayQuery = rayQueryFeatures.rayQuery,

      .accelerationStructure =
          accelerationStructureFeatures.accelerationStructure,
      .accelerationStructureCaptureReplay =
          accelerationStructureFeatures.accelerationStructureCaptureReplay,
      .accelerationStructureIndirectBuild =
          accelerationStructureFeatures.accelerationStructureIndirectBuild,
      .descriptorBindingAccelerationStructureUpdateAfterBind =
          accelerationStructureFeatures
              .descriptorBindingAccelerationStructureUpdateAfterBind,
      .deferredHostOperations = deferredHostOperationsExt,
      .descriptorHeap = descriptorHeapFeatures.descriptorHeap,
      .descriptorHeapCaptureReplay =
          descriptorHeapFeatures.descriptorHeapCaptureReplay,
  };
}
} // namespace strobe::rhi::vulkan
