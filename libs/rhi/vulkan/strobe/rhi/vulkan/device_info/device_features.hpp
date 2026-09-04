#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/vulkan/device_info/device_extensions.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct DeviceFeatures {
  // VK_API_VERSION_1_0
  VkBool32 robustBufferAccess;
  VkBool32 fullDrawIndexUint32;
  VkBool32 imageCubeArray;
  VkBool32 independentBlend;
  VkBool32 geometryShader;
  VkBool32 tessellationShader;
  VkBool32 sampleRateShading;
  VkBool32 dualSrcBlend;
  VkBool32 logicOp;
  VkBool32 multiDrawIndirect;
  VkBool32 drawIndirectFirstInstance;
  VkBool32 depthClamp;
  VkBool32 depthBiasClamp;
  VkBool32 fillModeNonSolid;
  VkBool32 depthBounds;
  VkBool32 wideLines;
  VkBool32 largePoints;
  VkBool32 alphaToOne;
  VkBool32 multiViewport;
  VkBool32 samplerAnisotropy;
  VkBool32 textureCompressionETC2;
  VkBool32 textureCompressionASTC_LDR;
  VkBool32 textureCompressionBC;
  VkBool32 occlusionQueryPrecise;
  VkBool32 pipelineStatisticsQuery;
  VkBool32 vertexPipelineStoresAndAtomics;
  VkBool32 fragmentStoresAndAtomics;
  VkBool32 shaderTessellationAndGeometryPointSize;
  VkBool32 shaderImageGatherExtended;
  VkBool32 shaderStorageImageExtendedFormats;
  VkBool32 shaderStorageImageMultisample;
  VkBool32 shaderStorageImageReadWithoutFormat;
  VkBool32 shaderStorageImageWriteWithoutFormat;
  VkBool32 shaderUniformBufferArrayDynamicIndexing;
  VkBool32 shaderSampledImageArrayDynamicIndexing;
  VkBool32 shaderStorageBufferArrayDynamicIndexing;
  VkBool32 shaderStorageImageArrayDynamicIndexing;
  VkBool32 shaderClipDistance;
  VkBool32 shaderCullDistance;
  VkBool32 shaderFloat64;
  VkBool32 shaderInt64;
  VkBool32 shaderInt16;
  VkBool32 shaderResourceResidency;
  VkBool32 shaderResourceMinLod;
  VkBool32 sparseBinding;
  VkBool32 sparseResidencyBuffer;
  VkBool32 sparseResidencyImage2D;
  VkBool32 sparseResidencyImage3D;
  VkBool32 sparseResidency2Samples;
  VkBool32 sparseResidency4Samples;
  VkBool32 sparseResidency8Samples;
  VkBool32 sparseResidency16Samples;
  VkBool32 sparseResidencyAliased;
  VkBool32 variableMultisampleRate;
  VkBool32 inheritedQueries;
  // VK_API_VERSION_1_1
  VkBool32 storageBuffer16BitAccess;
  VkBool32 uniformAndStorageBuffer16BitAccess;
  VkBool32 storagePushConstant16;
  VkBool32 storageInputOutput16;
  VkBool32 multiview;
  VkBool32 multiviewGeometryShader;
  VkBool32 multiviewTessellationShader;
  VkBool32 variablePointersStorageBuffer;
  VkBool32 variablePointers;
  VkBool32 protectedMemory;
  VkBool32 samplerYcbcrConversion;
  VkBool32 shaderDrawParameters;
  // VK_API_VERSION_1_2
  VkBool32 samplerMirrorClampToEdge;
  VkBool32 drawIndirectCount;
  VkBool32 storageBuffer8BitAccess;
  VkBool32 uniformAndStorageBuffer8BitAccess;
  VkBool32 storagePushConstant8;
  VkBool32 shaderBufferInt64Atomics;
  VkBool32 shaderSharedInt64Atomics;
  VkBool32 shaderFloat16;
  VkBool32 shaderInt8;
  VkBool32 descriptorIndexing;
  VkBool32 shaderInputAttachmentArrayDynamicIndexing;
  VkBool32 shaderUniformTexelBufferArrayDynamicIndexing;
  VkBool32 shaderStorageTexelBufferArrayDynamicIndexing;
  VkBool32 shaderUniformBufferArrayNonUniformIndexing;
  VkBool32 shaderSampledImageArrayNonUniformIndexing;
  VkBool32 shaderStorageBufferArrayNonUniformIndexing;
  VkBool32 shaderStorageImageArrayNonUniformIndexing;
  VkBool32 shaderInputAttachmentArrayNonUniformIndexing;
  VkBool32 shaderUniformTexelBufferArrayNonUniformIndexing;
  VkBool32 shaderStorageTexelBufferArrayNonUniformIndexing;
  VkBool32 descriptorBindingUniformBufferUpdateAfterBind;
  VkBool32 descriptorBindingSampledImageUpdateAfterBind;
  VkBool32 descriptorBindingStorageImageUpdateAfterBind;
  VkBool32 descriptorBindingStorageBufferUpdateAfterBind;
  VkBool32 descriptorBindingUniformTexelBufferUpdateAfterBind;
  VkBool32 descriptorBindingStorageTexelBufferUpdateAfterBind;
  VkBool32 descriptorBindingUpdateUnusedWhilePending;
  VkBool32 descriptorBindingPartiallyBound;
  VkBool32 descriptorBindingVariableDescriptorCount;
  VkBool32 runtimeDescriptorArray;
  VkBool32 samplerFilterMinmax;
  VkBool32 scalarBlockLayout;
  VkBool32 imagelessFramebuffer;
  VkBool32 uniformBufferStandardLayout;
  VkBool32 shaderSubgroupExtendedTypes;
  VkBool32 separateDepthStencilLayouts;
  VkBool32 hostQueryReset;
  VkBool32 timelineSemaphore;
  VkBool32 bufferDeviceAddress;
  VkBool32 bufferDeviceAddressCaptureReplay;
  VkBool32 bufferDeviceAddressMultiDevice;
  VkBool32 vulkanMemoryModel;
  VkBool32 vulkanMemoryModelDeviceScope;
  VkBool32 vulkanMemoryModelAvailabilityVisibilityChains;
  VkBool32 shaderOutputViewportIndex;
  VkBool32 shaderOutputLayer;
  VkBool32 subgroupBroadcastDynamicId;
  // VK_API_VERSION_1_3
  VkBool32 robustImageAccess;
  VkBool32 inlineUniformBlock;
  VkBool32 descriptorBindingInlineUniformBlockUpdateAfterBind;
  VkBool32 pipelineCreationCacheControl;
  VkBool32 privateData;
  VkBool32 shaderDemoteToHelperInvocation;
  VkBool32 shaderTerminateInvocation;
  VkBool32 subgroupSizeControl;
  VkBool32 computeFullSubgroups;
  VkBool32 synchronization2;
  VkBool32 textureCompressionASTC_HDR;
  VkBool32 shaderZeroInitializeWorkgroupMemory;
  VkBool32 dynamicRendering;
  VkBool32 shaderIntegerDotProduct;
  VkBool32 maintenance4;
  // VK_API_VERSION_1_4
  VkBool32 globalPriorityQuery;
  VkBool32 shaderSubgroupRotate;
  VkBool32 shaderSubgroupRotateClustered;
  VkBool32 shaderFloatControls2;
  VkBool32 shaderExpectAssume;
  VkBool32 rectangularLines;
  VkBool32 bresenhamLines;
  VkBool32 smoothLines;
  VkBool32 stippledRectangularLines;
  VkBool32 stippledBresenhamLines;
  VkBool32 stippledSmoothLines;
  VkBool32 vertexAttributeInstanceRateDivisor;
  VkBool32 vertexAttributeInstanceRateZeroDivisor;
  VkBool32 indexTypeUint8;
  VkBool32 dynamicRenderingLocalRead;
  VkBool32 maintenance5;
  VkBool32 maintenance6;
  VkBool32 pipelineProtectedAccess;
  VkBool32 pipelineRobustness;
  VkBool32 hostImageCopy;
  VkBool32 pushDescriptor;

  VkBool32 swapchainMaintenance1;
  VkBool32 shaderObjects;

  VkBool32 calibratedTimestamps;

  // raytracing pipeline
  VkBool32 rayTracingPipeline;
  VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplay;
  VkBool32 rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
  VkBool32 rayTracingPipelineTraceRaysIndirect;
  VkBool32 rayTraversalPrimitiveCulling;
  // VK_KHR_ray_tracing_maintenance1
  VkBool32 rayTracingMaintenance1;
  VkBool32 rayTracingPipelineTraceRaysIndirect2;
  // VK_KHR_ray_query
  VkBool32 rayQuery;

  // acceleration structure
  VkBool32 accelerationStructure;
  VkBool32 accelerationStructureCaptureReplay;
  VkBool32 accelerationStructureIndirectBuild;
  VkBool32 descriptorBindingAccelerationStructureUpdateAfterBind;

  // deferred host operations
  VkBool32 deferredHostOperations;

  // descriptor heaps
  VkBool32 descriptorHeap;
  VkBool32 descriptorHeapCaptureReplay;

  // maintenance 9
  VkBool32 maintenance9;
  
  // shader untyped pointers
  VkBool32 shaderUntypedPointers;
};

namespace details {

DeviceFeatures
query_device_features(VkPhysicalDevice physicalDevice, uint32_t apiVersion,
                      span<const DeviceExtension> supportedExtensions) noexcept;

} // namespace details

} // namespace strobe::rhi::vulkan
