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

vulkan::ShaderObject create_shader_object(const Context &context,
                                          ShaderStage stage,
                                          ShaderStage nextStage,
                                          span<const uint32_t> spirv) {
  return vulkan::create_shader_object(
      context.ctx(),
      {
          .stage =
              static_cast<VkShaderStageFlagBits>(to_vk_shader_stage(stage)),
          .flags = 0,
          .nextStage =
              static_cast<VkShaderStageFlagBits>(to_vk_shader_stage(nextStage)),
          .spirv = spirv,
          .specInfo = nullptr,
      });
}

VertexShader create_vertex(Context context, const VertexShaderInfo &info,
                           handle_allocators *alloc) {
  ZoneScopedN("shader/create-vertex");
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::vertex, info.nextStage,
                           info.spirv);
  return VertexShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

FragmentShader create_fragment(Context context, const FragmentShaderInfo &info,
                               handle_allocators *alloc) {
  ZoneScopedN("shader/create-fragment");
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::fragment, ShaderStage::none,
                           info.spirv);
  return FragmentShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

ComputeShader create_compute(Context context, const ComputeShaderInfo &info,
                             handle_allocators *alloc) {
  ZoneScopedN("shader/create-compute");
  const vulkan::ShaderObject so =
      create_shader_object(context, ShaderStage::compute, ShaderStage::none,
                           info.spirv);
  return ComputeShader{make_void_handle<ShaderObjectImpl>(
      &alloc->shaderAlloc, std::move(context), so)};
}

} // namespace strobe::rhi::shader
