#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/pipeline_cache.hpp"
#include "strobe/gpu/vulkan/pipeline_layout.hpp"
#include "strobe/gpu/vulkan/shader_module.hpp"
#include "strobe/gpu/vulkan/specialization_info.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct ComputePipeline {
  VkPipeline handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct ComputePipelineInfo {
  VkPipelineCreateFlags flags = 0;
  PipelineLayout layout{};
  ShaderModule module{};
  const char *entry = "main";
  SpecializationInfo specializationInfo;
  VkPipelineShaderStageCreateFlags stageFlags = 0;
  PipelineCache cache{};

  uint32_t subgroupSize = 0;
};

ComputePipeline create_compute_pipeline(Context *context,
                                        const ComputePipelineInfo &info);

void destroy_compute_pipeline(Context *context,
                              ComputePipeline pipeline) noexcept;

} // namespace strobe::gpu::vulkan
