#include "strobe/gpu/vulkan/compute_pipeline.hpp"

#include <stdexcept>

namespace strobe::gpu::vulkan {

ComputePipeline create_compute_pipeline(Context *context,
                                        const ComputePipelineInfo &info) {
  assert(context != nullptr);
  assert(info.layout);
  assert(info.module);
  assert(info.entry != nullptr);

  static constexpr size_t SCRATCH_SIZE = sizeof(VkSpecializationMapEntry) * 8;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  uint32_t specCount = info.specializationInfo.entries.size();
  Vector<VkSpecializationMapEntry, scratch_allocator_ref> specEntries{specCount,
                                                                      &scratch};
  for (uint32_t i = 0; i < specCount; ++i) {
    specEntries[i] = VkSpecializationMapEntry{
        .constantID = info.specializationInfo.entries[i].id,
        .offset = info.specializationInfo.entries[i].offset,
        .size = info.specializationInfo.entries[i].size,
    };
  }

  VkSpecializationInfo specInfo{
      .mapEntryCount = static_cast<uint32_t>(specCount),
      .pMapEntries = specEntries.data(),
      .dataSize = info.specializationInfo.data.size(),
      .pData = info.specializationInfo.data.data(),
  };

  VkSpecializationInfo *pSpecInfo = nullptr;
  if (specCount != 0) {
    assert(!info.specializationInfo.data.empty());
    pSpecInfo = &specInfo;
  }

  void *pNextStage = nullptr;

  assert(info.subgroupSize == 0 ||
         !(info.stageFlags &
           VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT));

  VkPipelineShaderStageRequiredSubgroupSizeCreateInfo subgroupInfo{
      .sType =
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO,
      .pNext = pNextStage,
      .requiredSubgroupSize = info.subgroupSize,
  };
  if (info.subgroupSize != 0) {
    assert(context->properties().subgroup_control);
    pNextStage = &subgroupInfo;
  }

  const VkComputePipelineCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .stage =
          VkPipelineShaderStageCreateInfo{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
              .pNext = pNextStage,
              .flags = info.stageFlags,
              .stage = VK_SHADER_STAGE_COMPUTE_BIT,
              .module = info.module.handle,
              .pName = info.entry,
              .pSpecializationInfo = pSpecInfo,
          },
      .layout = info.layout.handle,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };

  ComputePipeline pipeline{};

  const VkResult result = vkCreateComputePipelines(
      context->device(), info.cache.handle, 1, &createInfo,
      context->driver_alloc(), &pipeline.handle);

  if (result != VK_SUCCESS) {
    if (pipeline) {
      vkDestroyPipeline(context->device(), pipeline.handle,
                        context->driver_alloc());
    }

    throw std::runtime_error("Failed to create compute pipeline");
  }

  return pipeline;
}

void destroy_compute_pipeline(Context *context,
                              ComputePipeline pipeline) noexcept {
  assert(context != nullptr);
  assert(pipeline);
  vkDestroyPipeline(context->device(), pipeline.handle,
                    context->driver_alloc());
}
} // namespace strobe::gpu::vulkan
