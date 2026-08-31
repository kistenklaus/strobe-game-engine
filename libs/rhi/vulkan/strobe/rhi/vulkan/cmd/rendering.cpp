#include "strobe/rhi/vulkan/cmd/rendering.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/utils/blend_factor_utils.hpp"
#include "strobe/rhi/utils/blend_op_utils.hpp"
#include "strobe/rhi/utils/color_component_utils.hpp"
#include "strobe/rhi/utils/compare_op_utils.hpp"
#include "strobe/rhi/utils/cull_mode_utils.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/front_face_utils.hpp"
#include "strobe/rhi/utils/logic_op_utils.hpp"
#include "strobe/rhi/utils/polygon_mode_utils.hpp"
#include "strobe/rhi/utils/primitive_topology_utils.hpp"
#include "strobe/rhi/utils/sample_count_utils.hpp"
#include "strobe/rhi/utils/shader_stage_utils.hpp"
#include "strobe/rhi/utils/stencil_face_utils.hpp"
#include "strobe/rhi/utils/stencil_op_utils.hpp"
#include "strobe/rhi/utils/vertex_input_rate_utils.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <vk_video/vulkan_video_codec_av1std.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void cmd_begin_rendering(CommandBuffer cmd,
                         const RenderingInfo &info) noexcept {
  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext = nullptr,
      .flags = 0,
      .renderArea =
          {
              .offset = {info.renderArea.offset.x(),
                         info.renderArea.offset.y()},
              .extent = {info.renderArea.extent.x(),
                         info.renderArea.extent.y()},
          },
      .layerCount = info.layerCount,
      .viewMask = info.viewMask,
      .colorAttachmentCount =
          static_cast<uint32_t>(info.colorAttachments.size()),
      .pColorAttachments = info.colorAttachments.data(),
      .pDepthAttachment = info.depthAttachment.has_value()
                              ? &info.depthAttachment.value()
                              : nullptr,
      .pStencilAttachment = info.stencilAttachment.has_value()
                                ? &info.stencilAttachment.value()
                                : nullptr,
  };

  {
    ZoneScopedN("vkCmdBeginRendering");
    vkCmdBeginRendering(cmd.handle, &renderingInfo);
  }
}
void cmd_end_rendering(CommandBuffer cmd) noexcept {
  {
    ZoneScopedN("vkCmdEndRendering");
    vkCmdEndRendering(cmd.handle);
  }
}
void cmd_set_viewports(CommandBuffer cmd,
                       span<const Viewport> viewports) noexcept {
  SmallVector<VkViewport, 4> vk{viewports.size()};
  for (uint32_t i = 0; i < viewports.size(); ++i) {
    vk[i] = VkViewport{
        .x = viewports[i].position.x(),
        .y = viewports[i].position.y(),
        .width = viewports[i].extent.x(),
        .height = viewports[i].extent.y(),
        .minDepth = viewports[i].minDepth,
        .maxDepth = viewports[i].maxDepth,
    };
  }
  cmd_set_viewports(cmd, vk);
}
void cmd_set_viewports(CommandBuffer cmd,
                       span<const VkViewport> viewports) noexcept {
  {
    ZoneScopedN("vkCmdSetViewportWithCount");
    vkCmdSetViewportWithCount(cmd.handle, viewports.size(), viewports.data());
  }
}

void cmd_set_scissors(CommandBuffer cmd, span<const Rect> scissors) noexcept {
  SmallVector<VkRect2D, 4> vk{scissors.size()};
  for (uint32_t i = 0; i < scissors.size(); ++i) {
    vk[i] = VkRect2D{
        .offset = {scissors[i].offset.x(), scissors[i].offset.y()},
        .extent = {scissors[i].extent.x(), scissors[i].extent.y()},
    };
  }
  cmd_set_scissors(cmd, vk);
}
void cmd_set_scissors(CommandBuffer cmd,
                      span<const VkRect2D> scissors) noexcept {
  {
    ZoneScopedN("vkCmdSetScissorWithCount");
    vkCmdSetScissorWithCount(cmd.handle, scissors.size(), scissors.data());
  }
}

void cmd_set_rasterizer_discard_enable(CommandBuffer cmd,
                                       bool discardEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetRasterizerDiscardEnable");
    vkCmdSetRasterizerDiscardEnable(cmd.handle, discardEnable);
  }
}
void cmd_set_primitive_topology(CommandBuffer cmd,
                                PrimitiveTopology topology) noexcept {
  cmd_set_primitive_topology(cmd, to_vk_primitive_topology(topology));
}
void cmd_set_primitive_topology(CommandBuffer cmd,
                                VkPrimitiveTopology topology) noexcept {
  {
    ZoneScopedN("vkCmdSetPrimitiveTopology");
    vkCmdSetPrimitiveTopology(cmd.handle, topology);
  }
}

void cmd_set_primitive_restart(CommandBuffer cmd, bool restartEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetPrimitiveRestartEnable");
    vkCmdSetPrimitiveRestartEnable(cmd.handle, restartEnable);
  }
}
void cmd_set_cull_mode(CommandBuffer cmd, CullMode cullMode) noexcept {
  cmd_set_cull_mode(cmd, to_vk_cull_mode(cullMode));
}

void cmd_set_cull_mode(CommandBuffer cmd, VkCullModeFlags cullMode) noexcept {
  {
    ZoneScopedN("vkCmdSetCullMode");
    vkCmdSetCullMode(cmd.handle, cullMode);
  }
}

void cmd_set_front_face(CommandBuffer cmd, FrontFace frontFace) noexcept {
  cmd_set_front_face(cmd, to_vk_front_face(frontFace));
}

void cmd_set_front_face(CommandBuffer cmd, VkFrontFace frontFace) noexcept {
  {
    ZoneScopedN("vkCmdSetFrontFace");
    vkCmdSetFrontFace(cmd.handle, frontFace);
  }
}

void cmd_set_depth_bias_enable(CommandBuffer cmd,
                               bool depthBiasEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthBiasEnable");
    vkCmdSetDepthBiasEnable(cmd.handle, depthBiasEnable);
  }
}
void cmd_set_depth_test_enable(CommandBuffer cmd,
                               bool depthTestEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthTestEnable");
    vkCmdSetDepthTestEnable(cmd.handle, depthTestEnable);
  }
}
void cmd_set_depth_write_enable(CommandBuffer cmd,
                                bool depthWriteEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthWriteEnable");
    vkCmdSetDepthWriteEnable(cmd.handle, depthWriteEnable);
  }
}
void cmd_set_depth_compare_op(CommandBuffer cmd, CompareOp op) noexcept {
  cmd_set_depth_compare_op(cmd, to_vk_compare_op(op));
}
void cmd_set_depth_compare_op(CommandBuffer cmd, VkCompareOp op) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthCompareOp");
    vkCmdSetDepthCompareOp(cmd.handle, op);
  }
}
void cmd_set_depth_bounds_test_enable(CommandBuffer cmd,
                                      bool depthBoundsTestEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthBoundsTestEnable");
    vkCmdSetDepthBoundsTestEnable(cmd.handle, depthBoundsTestEnable);
  }
}

void cmd_set_depth_bounds(CommandBuffer cmd, float minDepthBound,
                          float maxDepthBound) noexcept {
  {
    ZoneScopedN("vkCmdSetDepthBounds");
    vkCmdSetDepthBounds(cmd.handle, minDepthBound, maxDepthBound);
  }
}
void cmd_set_stencil_test_enable(CommandBuffer cmd,
                                 bool stencilTestEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetStencilTestEnable");
    vkCmdSetStencilTestEnable(cmd.handle, stencilTestEnable);
  }
}
void cmd_set_stencil_op(CommandBuffer cmd, StencilFace faceMask,
                        StencilOp failOp, StencilOp passOp,
                        StencilOp depthFailOp, CompareOp compareOp) noexcept {
  cmd_set_stencil_op(cmd, to_vk_stencil_face(faceMask),
                     to_vk_stencil_op(failOp), to_vk_stencil_op(passOp),
                     to_vk_stencil_op(depthFailOp),
                     to_vk_compare_op(compareOp));
}

void cmd_set_stencil_op(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                        VkStencilOp failOp, VkStencilOp passOp,
                        VkStencilOp depthFailOp,
                        VkCompareOp compareOp) noexcept {
  {
    ZoneScopedN("vkCmdSetStencilOp");
    vkCmdSetStencilOp(cmd.handle, faceMask, failOp, passOp, depthFailOp,
                      compareOp);
  }
}
void cmd_set_stencil_compare_mask(CommandBuffer cmd,
                                  VkStencilFaceFlags faceMask,
                                  uint32_t compareMask) noexcept {
  {
    ZoneScopedN("vkCmdSetStencilCompareMask");
    vkCmdSetStencilCompareMask(cmd.handle, faceMask, compareMask);
  }
}
void cmd_set_stencil_compare_mask(CommandBuffer cmd, StencilFace faceMask,
                                  uint32_t compareMask) noexcept {
  cmd_set_stencil_compare_mask(cmd, to_vk_stencil_face(faceMask), compareMask);
}
void cmd_set_stencil_write_mask(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                                uint32_t writeMask) noexcept {
  {
    ZoneScopedN("vkCmdSetStencilWriteMask");
    vkCmdSetStencilWriteMask(cmd.handle, faceMask, writeMask);
  }
}
void cmd_set_stencil_write_mask(CommandBuffer cmd, StencilFace faceMask,
                                uint32_t writeMask) noexcept {
  cmd_set_stencil_write_mask(cmd, to_vk_stencil_face(faceMask), writeMask);
}
void cmd_set_stencil_reference(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                               uint32_t reference) noexcept {
  {
    ZoneScopedN("vkCmdSetStencilReference");
    vkCmdSetStencilReference(cmd.handle, faceMask, reference);
  }
}

void cmd_set_stencil_reference(CommandBuffer cmd, StencilFace faceMask,
                               uint32_t reference) noexcept {
  cmd_set_stencil_write_mask(cmd, to_vk_stencil_face(faceMask), reference);
}
void cmd_set_blend_constants(CommandBuffer cmd, vec4 constants) noexcept {
  float blendConstants[4] = {
      constants.x(),
      constants.y(),
      constants.z(),
      constants.w(),
  };
  {
    ZoneScopedN("vkCmdSetBlendConstants");
    vkCmdSetBlendConstants(cmd.handle, blendConstants);
  }
}
void cmd_set_line_width(CommandBuffer cmd, float lineWidth) noexcept {
  {
    ZoneScopedN("vkCmdSetLineWidth");
    vkCmdSetLineWidth(cmd.handle, lineWidth);
  }
}
void cmd_set_depth_bias(CommandBuffer cmd, float depthBiasConstantFactor,
                        float depthBiasClamp,
                        float depthBiasSlopeFactor) noexcept {
  {
    vkCmdSetDepthBias(cmd.handle, depthBiasConstantFactor, depthBiasClamp,
                      depthBiasSlopeFactor);
  }
}
void cmd_set_vertex_input(
    const Context *context, CommandBuffer cmd,
    span<const VkVertexInputBindingDescription2EXT> bindings,
    span<const VkVertexInputAttributeDescription2EXT> attributes) noexcept {
  {
    ZoneScopedN("vkCmdSetVertexInput");
    vulkan::vk_cmd_set_vertex_input(context->pnf(), cmd.handle, bindings.size(),
                                    bindings.data(), attributes.size(),
                                    attributes.data());
  }
}
void cmd_set_vertex_input(const Context *context, CommandBuffer cmd,
                          span<const VertexBinding> bindings,
                          span<const VertexAttribute> attributes) noexcept {
  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkVertexInputBindingDescription2EXT) * 8 +
      sizeof(VkVertexInputAttributeDescription2EXT) * 8;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, SCRATCH_SIZE>;
  scratch_allocator scratch{};
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  Vector<VkVertexInputBindingDescription2EXT, scratch_allocator_ref>
      bindingDescriptions{bindings.size(), &scratch};
  for (uint32_t i = 0; i < bindings.size(); ++i) {
    const auto &binding = bindings[i];
    bindingDescriptions[i] = VkVertexInputBindingDescription2EXT{
        .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
        .pNext = nullptr,
        .binding = binding.binding,
        .stride = binding.stride,
        .inputRate = to_vk_vertex_input_rate(binding.inputRate),
        .divisor = binding.divisor,
    };
  }
  Vector<VkVertexInputAttributeDescription2EXT, scratch_allocator_ref>
      attributeDescriptions{attributes.size(), &scratch};
  for (uint32_t i = 0; i < attributes.size(); ++i) {
    const auto &attrib = attributes[i];
    attributeDescriptions[i] = VkVertexInputAttributeDescription2EXT{
        .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
        .pNext = nullptr,
        .location = attrib.location,
        .binding = attrib.binding,
        .format = to_vk_format(attrib.format),
        .offset = attrib.offset,
    };
  }
  cmd_set_vertex_input(context, cmd, bindingDescriptions,
                       attributeDescriptions);
}
void cmd_set_rasterizer_samples(const Context *context, CommandBuffer cmd,
                                VkSampleCountFlagBits samples) noexcept {
  {
    ZoneScopedN("vkCmdSetRasterizerSamples");
    vulkan::vk_cmd_set_rasterization_samples(context->pnf(), cmd.handle,
                                             samples);
  }
}
void cmd_set_rasterizer_samples(const Context *context, CommandBuffer cmd,
                                SampleCount samples) noexcept {
  cmd_set_rasterizer_samples(context, cmd, to_vk_sample_count(samples));
}


void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         SampleCount samples, uint64_t mask) noexcept {
  const VkSampleCountFlagBits sampleCount = to_vk_sample_count(samples);
  const VkSampleMask masks[2] = {
      static_cast<uint32_t>(mask),
      static_cast<uint32_t>(mask >> 32),
  };
  cmd_set_sample_mask(context, cmd, sampleCount, masks);
}

void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         VkSampleCountFlagBits samples,
                         const VkSampleMask mask[2]) noexcept {
  {
    ZoneScopedN("vkCmdSetSampleMask");
    vulkan::vk_cmd_set_sample_mask(context->pnf(), cmd.handle, samples, mask);
  }
}
void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         VkSampleCountFlagBits samples,
                         uint64_t mask) noexcept {
  const VkSampleMask masks[2] = {
      static_cast<uint32_t>(mask),
      static_cast<uint32_t>(mask >> 32),
  };
  {
    ZoneScopedN("vkCmdSetSampleMask");
    vulkan::vk_cmd_set_sample_mask(context->pnf(), cmd.handle, samples, masks);
  }
}

void cmd_set_alpha_to_coverage_enable(const Context *context, CommandBuffer cmd,
                                      bool alphaToCoverage) noexcept {
  {
    ZoneScopedN("vkCmdSetAlphaToCoverageEnable");
    vulkan::vk_cmd_set_alpha_to_coverage_enable(context->pnf(), cmd.handle,
                                                alphaToCoverage);
  }
}
void cmd_set_polygon_mode(const Context *context, CommandBuffer cmd,
                          VkPolygonMode polygonMode) noexcept {
  {
    ZoneScopedN("vkCmdSetPolygonMode");
    vulkan::vk_cmd_set_polygon_mode(context->pnf(), cmd.handle, polygonMode);
  }
}
void cmd_set_polygon_mode(const Context *context, CommandBuffer cmd,
                          PolygonMode polygonMode) noexcept {
  cmd_set_polygon_mode(context, cmd, to_vk_polygon_mode(polygonMode));
}


void cmd_set_depth_clamp_enable(const Context *context, CommandBuffer cmd,
                                bool depthClampEnable) {
  {
    ZoneScopedN("vkCmdSetDepthClampEnable");
    vulkan::vk_cmd_set_depth_clamp_enable(context->pnf(), cmd.handle,
                                          depthClampEnable);
  }
}
void cmd_set_logic_op_enable(const Context *context, CommandBuffer cmd,
                             bool logicOpEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetLogicOpEnable");
    vulkan::vk_cmd_set_logic_op_enable(context->pnf(), cmd.handle,
                                       logicOpEnable);
  }
}

void cmd_set_logic_op(const Context *context, CommandBuffer cmd,
                      VkLogicOp logicOp) noexcept {
  {
    ZoneScopedN("vkCmdSetLogicOp");
    vulkan::vk_cmd_set_logic_op(context->pnf(), cmd.handle, logicOp);
  }
}
void cmd_set_logic_op(const Context *context, CommandBuffer cmd,
                      LogicOp logicOp) noexcept {
  cmd_set_logic_op(context, cmd, to_vk_logic_op(logicOp));
}

void cmd_set_color_blend_enable(const Context *context, CommandBuffer cmd,
                                uint32_t firstAttachment,
                                uint32_t attachmentCount,
                                uint32_t bitmask) noexcept {
  assert(attachmentCount <= 32);
  VkBool32 enable[32]{};
  for (uint32_t i = 0; i < attachmentCount; ++i) {
    enable[i] = (bitmask & (uint32_t{1} << i)) ? VK_TRUE : VK_FALSE;
  }
  {
    ZoneScopedN("vkCmdSetColorBlendEnable");
    vulkan::vk_cmd_set_color_blend_enable(
        context->pnf(), cmd.handle, firstAttachment, attachmentCount, enable);
  }
}
void cmd_set_color_blend_equation(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const VkColorBlendEquationEXT> colorBlendEquations) noexcept {
  {
    ZoneScopedN("vkCmdSetColorBlendEquation");
    vulkan::vk_cmd_set_color_blend_equation(
        context->pnf(), cmd.handle, firstAttachment, colorBlendEquations.size(),
        colorBlendEquations.data());
  }
}
void cmd_set_color_blend_equation(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const BlendEquation> colorBlendEquations) noexcept {
  SmallVector<VkColorBlendEquationEXT, 4> equations{colorBlendEquations.size()};
  for (uint32_t i = 0; i < colorBlendEquations.size(); ++i) {
    const auto &eq = colorBlendEquations[i];
    equations[i] = VkColorBlendEquationEXT{
        .srcColorBlendFactor = to_vk_blend_factor(eq.srcColor),
        .dstColorBlendFactor = to_vk_blend_factor(eq.dstColor),
        .colorBlendOp = to_vk_blend_op(eq.colorBlendOp),
        .srcAlphaBlendFactor = to_vk_blend_factor(eq.srcAlpha),
        .dstAlphaBlendFactor = to_vk_blend_factor(eq.dstAlpha),
        .alphaBlendOp = to_vk_blend_op(eq.alphaBlendOp),
    };
  }
  cmd_set_color_blend_equation(context, cmd, firstAttachment, equations);
}
void cmd_set_color_write_mask(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const VkColorComponentFlags> colorWriteMasks) noexcept {
  {
    ZoneScopedN("vkCmdSetColorWriteMask");
    vulkan::vk_cmd_set_color_write_mask(context->pnf(), cmd.handle,
                                        firstAttachment, colorWriteMasks.size(),
                                        colorWriteMasks.data());
  }
}
void cmd_set_color_write_mask(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const ColorComponent> colorWriteMasks) noexcept {
  SmallVector<VkColorComponentFlags, 4> masks(colorWriteMasks.size());
  for (uint32_t i = 0; i < masks.size(); ++i) {
    const auto &mask = colorWriteMasks[i];
    masks[i] = to_vk_color_component(mask);
  }
  cmd_set_color_write_mask(context, cmd, firstAttachment, masks);
}
void cmd_set_alpha_to_one_enable(const Context *context, CommandBuffer cmd,
                                 bool alphaToOneEnable) noexcept {
  {
    ZoneScopedN("vkCmdSetAlphaToOneEnable");
    vulkan::vk_cmd_set_alpha_to_one_enable(context->pnf(), cmd.handle,
                                           alphaToOneEnable);
  }
}
void cmd_set_patch_control_points(const Context *context, CommandBuffer cmd,
                                  uint32_t patchControlPoints) noexcept {
  ZoneScopedN("vkCmdSetPatchControlPoints");
  vulkan::vk_cmd_set_patch_control_points(context->pnf(), cmd.handle,
                                          patchControlPoints);
}
void cmd_bind_vertex_buffer(CommandBuffer cmd,
                            BufferOffset verticies) noexcept {
  {
    ZoneScopedN("vkCmdBindVertexBuffers");
    vkCmdBindVertexBuffers(cmd.handle, 0, 1, &verticies.buffer.handle,
                           &verticies.offset);
  }
}

void cmd_bind_shader(const Context *context, CommandBuffer cmd,
                     ShaderObject shader,
                     VkShaderStageFlagBits stage) noexcept {
  {
    ZoneScopedN("vkCmdBindShaders");
    vulkan::vk_cmd_bind_shaders(context->pnf(), cmd.handle, 1, &stage,
                                &shader.handle);
  }
}
void cmd_bind_shader(const Context *context, CommandBuffer cmd,
                     ShaderObject shader, ShaderStage stage) noexcept {
  cmd_bind_shader(
      context, cmd, shader,
      static_cast<VkShaderStageFlagBits>(to_vk_shader_stage(stage)));
}
void cmd_unbind_shaders(const Context *context, CommandBuffer cmd,
                        ShaderStage stages) noexcept {
  using bitmask = std::underlying_type_t<ShaderStage>;
  static_assert(static_cast<uint32_t>(ShaderStage::vertex) == (1u << 0));
  static_assert(static_cast<uint32_t>(ShaderStage::tessellation_control) ==
                (1u << 1));
  static_assert(static_cast<uint32_t>(ShaderStage::tessellation_evaluation) ==
                (1u << 2));
  static_assert(static_cast<uint32_t>(ShaderStage::geometry) == (1u << 3));
  static_assert(static_cast<uint32_t>(ShaderStage::fragment) == (1u << 4));
  static_assert(static_cast<uint32_t>(ShaderStage::compute) == (1u << 5));

  static_assert(static_cast<uint32_t>(ShaderStage::raygen) == (1u << 6));
  static_assert(static_cast<uint32_t>(ShaderStage::anyhit) == (1u << 7));
  static_assert(static_cast<uint32_t>(ShaderStage::hit) == (1u << 8));
  static_assert(static_cast<uint32_t>(ShaderStage::miss) == (1u << 9));
  static_assert(static_cast<uint32_t>(ShaderStage::intersection) == (1u << 10));
  static_assert(static_cast<uint32_t>(ShaderStage::callable) == (1u << 11));

  static_assert(static_cast<uint32_t>(ShaderStage::task) == (1u << 12));
  static_assert(static_cast<uint32_t>(ShaderStage::mesh) == (1u << 13));
  static constexpr VkShaderStageFlagBits SHADER_STAGE_LOOKUP[] = {
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
      VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
      VK_SHADER_STAGE_GEOMETRY_BIT,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      VK_SHADER_STAGE_COMPUTE_BIT,

      // Not valid for VK_EXT_shader_object:
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,
      VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM,

      VK_SHADER_STAGE_TASK_BIT_EXT,
      VK_SHADER_STAGE_MESH_BIT_EXT,
  };

  bitmask remaining = static_cast<bitmask>(stages);

  VkShaderStageFlagBits vkStages[14];
  uint32_t stageCount = 0;

  while (remaining != 0) {
    const uint32_t index = static_cast<uint32_t>(std::countr_zero(remaining));
    const VkShaderStageFlagBits vkStage = SHADER_STAGE_LOOKUP[index];
    assert(vkStage != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM &&
           "shader stage is not supported by shader objects");
    vkStages[stageCount++] = vkStage;
    remaining &= remaining - 1;
  }
  if (stageCount == 0) {
    return;
  }

  {
    ZoneScopedN("vkCmdBindShaders");
    vulkan::vk_cmd_bind_shaders(context->pnf(), cmd.handle, stageCount,
                                vkStages, nullptr);
  }
}

void cmd_draw(CommandBuffer cmd, uint32_t vertexCount, uint32_t instanceCount,
              uint32_t firstVertex, uint32_t firstInstance) noexcept {
  {
    ZoneScopedN("vkCmdDraw");
    vkCmdDraw(cmd.handle, vertexCount, instanceCount, firstVertex,
              firstInstance);
  }
}
void cmd_draw_indexed(CommandBuffer cmd, uint32_t indexCount,
                      uint32_t instanceCount, uint32_t firstIndex,
                      uint32_t vertexOffset, uint32_t firstInstance) noexcept {
  {
    ZoneScopedN("vkCmdDrawIndexed");
    vkCmdDrawIndexed(cmd.handle, indexCount, instanceCount, firstIndex,
                     vertexOffset, firstInstance);
  }
}

} // namespace strobe::rhi::vulkan
