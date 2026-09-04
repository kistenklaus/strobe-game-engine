#pragma once

#include <cstdint>
namespace strobe::rhi::vulkan {

struct ContextProperties {
  uint32_t api_version;

  // bitfields:
  uint32_t debug_utils : 1 = false;
  uint32_t surface : 1 = false;
  uint32_t swapchain : 1 = false;

  uint32_t timeline_semaphore : 1 = false;
  uint32_t synchronization2 : 1 = false;
  uint32_t dynamicRendering : 1 = false;
  uint32_t shaderObjects : 1 = false;

  uint32_t hostQueryReset : 1 = false;
  uint32_t calibratedTimestamps : 1 = false;
  uint32_t bufferDeviceAddress : 1 = false;

  uint32_t subgroup_control : 1 = false;

  uint32_t deviceAddress : 1 = false;

  uint32_t deferredHostOperations : 1 = false;
  uint32_t accelerationStructure : 1 = false;
  uint32_t raytracingPipeline : 1 = false;
  uint32_t raytracingPipelineMain1 : 1 = false;
  uint32_t rayQuery : 1 = false;

  uint32_t descriptorHeap : 1 = false;
  uint32_t shaderUntypedPointers : 1 = false;
};

} // namespace strobe::rhi::vulkan
