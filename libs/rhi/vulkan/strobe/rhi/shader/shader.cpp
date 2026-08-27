#include "strobe/rhi/shader/shader.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"
#include "strobe/rhi/types/push_constant_range.hpp"
#include "strobe/rhi/utils/shader_stage_utils.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::shader {

vulkan::ShaderObject
create_shader_object(const Context &context, ShaderStage stage,
                     ShaderStage nextStage, span<const uint32_t> spirv,
                     span<const PushConstantRange> pcRange) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 7>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};
  Vector<VkPushConstantRange, scratch_allocator_ref> pushConstantRange{
      pcRange.size(), &scratch};
  for (uint32_t i = 0; i < pcRange.size(); ++i) {
    pushConstantRange[i] = VkPushConstantRange{
        .stageFlags = to_vk_shader_stage(pcRange[i].stage),
        .offset = pcRange[i].offset,
        .size = pcRange[i].size,
    };
  }
  return vulkan::create_shader_object(
      context.ctx(),
      {
          .stage =
              static_cast<VkShaderStageFlagBits>(to_vk_shader_stage(stage)),
          .flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT,
          .nextStage =
              static_cast<VkShaderStageFlagBits>(to_vk_shader_stage(nextStage)),
          .spirv = spirv,
          .pushConstantRange = pushConstantRange,
          .specInfo = nullptr,
      });
}

VertexShader create(Context context, const VertexShaderInfo &info,
                    handle_allocators *alloc) {
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::vertex, info.nextStage,
                           info.spirv, info.pushConstantRange);
  return VertexShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

FragmentShader create(Context context, const FragmentShaderInfo &info,
                      handle_allocators *alloc) {
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::fragment, ShaderStage::none,
                           info.spirv, info.pushConstantRange);
  return FragmentShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

ComputeShader create(Context context, const ComputeShaderInfo &info,
                     handle_allocators *alloc) {
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::fragment, ShaderStage::none,
                           info.spirv, info.pushConstantRange);
  return ComputeShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

} // namespace strobe::rhi::shader
