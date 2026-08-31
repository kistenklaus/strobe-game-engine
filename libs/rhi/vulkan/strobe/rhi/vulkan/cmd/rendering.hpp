#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/blend_equation.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/compare_op.hpp"
#include "strobe/rhi/types/cull_mode.hpp"
#include "strobe/rhi/types/front_face.hpp"
#include "strobe/rhi/types/logic_op.hpp"
#include "strobe/rhi/types/polygon_mode.hpp"
#include "strobe/rhi/types/primitive_topology.hpp"
#include "strobe/rhi/types/rect.hpp"
#include "strobe/rhi/types/sample_count.hpp"
#include "strobe/rhi/types/shader_stage.hpp"
#include "strobe/rhi/types/stencil_face.hpp"
#include "strobe/rhi/types/stencil_op.hpp"
#include "strobe/rhi/types/vertex_attribute.hpp"
#include "strobe/rhi/types/vertex_binding.hpp"
#include "strobe/rhi/types/viewport.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/cmd/transfer.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/rhi/vulkan/shader_object.hpp"
#include <optional>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

struct RenderingInfo {
  Rect renderArea;
  uint32_t layerCount;
  uint32_t viewMask;
  span<const VkRenderingAttachmentInfo> colorAttachments;
  std::optional<VkRenderingAttachmentInfo> depthAttachment;
  std::optional<VkRenderingAttachmentInfo> stencilAttachment;
};

void cmd_begin_rendering(CommandBuffer cmd, const RenderingInfo &info) noexcept;

void cmd_end_rendering(CommandBuffer cmd) noexcept;

void cmd_set_viewports(CommandBuffer cmd,
                       span<const Viewport> viewports) noexcept;

void cmd_set_viewports(CommandBuffer cmd,
                       span<const VkViewport> viewports) noexcept;

void cmd_set_scissors(CommandBuffer cmd, span<const Rect> scissors) noexcept;

void cmd_set_scissors(CommandBuffer cmd,
                      span<const VkRect2D> scissors) noexcept;

void cmd_set_rasterizer_discard_enable(CommandBuffer cmd,
                                       bool discardEnable) noexcept;

void cmd_set_primitive_topology(CommandBuffer cmd,
                                PrimitiveTopology topology) noexcept;

void cmd_set_primitive_topology(CommandBuffer cmd,
                                VkPrimitiveTopology topology) noexcept;

void cmd_set_primitive_restart(CommandBuffer cmd, bool restartEnable) noexcept;

void cmd_set_cull_mode(CommandBuffer cmd, CullMode cullMode) noexcept;
void cmd_set_cull_mode(CommandBuffer cmd, VkCullModeFlags cullMode) noexcept;

void cmd_set_front_face(CommandBuffer cmd, FrontFace frontFace) noexcept;
void cmd_set_front_face(CommandBuffer cmd, VkFrontFace frontFace) noexcept;

void cmd_set_depth_bias_enable(CommandBuffer cmd,
                               bool depthBiasEnable) noexcept;

void cmd_set_depth_test_enable(CommandBuffer cmd,
                               bool depthTestEnable) noexcept;
void cmd_set_depth_write_enable(CommandBuffer cmd,
                                bool depthWriteEnable) noexcept;
void cmd_set_depth_compare_op(CommandBuffer cmd, CompareOp op) noexcept;
void cmd_set_depth_compare_op(CommandBuffer cmd, VkCompareOp op) noexcept;

void cmd_set_depth_bounds_test_enable(CommandBuffer cmd,
                                      bool depthBoundsTestEnable) noexcept;

void cmd_set_depth_bounds(CommandBuffer cmd, float minDepthBound,
                          float maxDepthBound) noexcept;

void cmd_set_stencil_test_enable(CommandBuffer cmd,
                                 bool stencilTestEnable) noexcept;

void cmd_set_stencil_op(CommandBuffer cmd, StencilFace faceMask,
                        StencilOp failOp, StencilOp passOp,
                        StencilOp depthFailOp, CompareOp compareOp) noexcept;
void cmd_set_stencil_op(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                        VkStencilOp failOp, VkStencilOp passOp,
                        VkStencilOp depthFailOp,
                        VkCompareOp compareOp) noexcept;

void cmd_set_stencil_compare_mask(CommandBuffer cmd,
                                  VkStencilFaceFlags faceMask,
                                  uint32_t compareMask) noexcept;
void cmd_set_stencil_compare_mask(CommandBuffer cmd, StencilFace faceMask,
                                  uint32_t compareMask) noexcept;

void cmd_set_stencil_write_mask(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                                uint32_t writeMask) noexcept;
void cmd_set_stencil_write_mask(CommandBuffer cmd, StencilFace faceMask,
                                uint32_t writeMask) noexcept;

void cmd_set_stencil_reference(CommandBuffer cmd, VkStencilFaceFlags faceMask,
                               uint32_t reference) noexcept;
void cmd_set_stencil_reference(CommandBuffer cmd, StencilFace faceMask,
                               uint32_t reference) noexcept;

void cmd_set_blend_constants(CommandBuffer cmd, vec4 constants) noexcept;

void cmd_set_line_width(CommandBuffer cmd, float lineWidth) noexcept;

void cmd_set_depth_bias(CommandBuffer cmd, float depthBiasConstantFactor,
                        float depthBiasClamp,
                        float depthBiasSlopeFactor) noexcept;

void cmd_set_vertex_input(
    const Context *context, CommandBuffer cmd,
    span<const VkVertexInputBindingDescription2EXT> bindings,
    span<const VkVertexInputAttributeDescription2EXT> attributes) noexcept;

void cmd_set_vertex_input(const Context *context, CommandBuffer cmd,
                          span<const VertexBinding> bindings,
                          span<const VertexAttribute> attributes) noexcept;

void cmd_set_rasterizer_samples(const Context *context, CommandBuffer cmd,
                                VkSampleCountFlagBits samples) noexcept;

void cmd_set_rasterizer_samples(const Context *context, CommandBuffer cmd,
                                SampleCount samples) noexcept;

void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         VkSampleCountFlagBits samples,
                         const VkSampleMask mask[2]) noexcept;

void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         SampleCount samples, uint64_t mask) noexcept;

void cmd_set_sample_mask(const Context *context, CommandBuffer cmd,
                         VkSampleCountFlagBits samples,
                         uint64_t mask) noexcept;

void cmd_set_alpha_to_coverage_enable(const Context *context, CommandBuffer cmd,
                                      bool alphaToCoverage) noexcept;

void cmd_set_polygon_mode(const Context *context, CommandBuffer cmd,
                          VkPolygonMode polygonMode) noexcept;
void cmd_set_polygon_mode(const Context *context, CommandBuffer cmd,
                          PolygonMode polygonMode) noexcept;

void cmd_set_depth_clamp_enable(const Context *context, CommandBuffer cmd,
                                bool depthClampEnable);

void cmd_set_logic_op_enable(const Context *context, CommandBuffer cmd,
                             bool logicOpEnable) noexcept;

void cmd_set_logic_op(const Context *context, CommandBuffer cmd,
                      VkLogicOp logicOp) noexcept;
void cmd_set_logic_op(const Context *context, CommandBuffer cmd,
                      LogicOp logicOp) noexcept;

void cmd_set_color_blend_enable(const Context *context, CommandBuffer cmd,
                                uint32_t firstAttachment,
                                uint32_t attachmentCount,
                                uint32_t bitmask) noexcept;

void cmd_set_color_blend_equation(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const VkColorBlendEquationEXT> colorBlendEquations) noexcept;

void cmd_set_color_blend_equation(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const BlendEquation> colorBlendEquations) noexcept;

void cmd_set_color_write_mask(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const VkColorComponentFlags> colorWriteMasks) noexcept;

void cmd_set_color_write_mask(
    const Context *context, CommandBuffer cmd, uint32_t firstAttachment,
    span<const ColorComponent> colorWriteMasks) noexcept;

void cmd_set_alpha_to_one_enable(const Context *context, CommandBuffer cmd,
                                 bool alphaToOneEnable) noexcept;

void cmd_set_patch_control_points(const Context *context, CommandBuffer cmd,
                                  uint32_t patchControlPoints) noexcept;

void cmd_bind_vertex_buffer(CommandBuffer cmd, BufferOffset buffer) noexcept;

void cmd_bind_shader(const Context *context, CommandBuffer cmd,
                     ShaderObject shader, VkShaderStageFlagBits stage) noexcept;

void cmd_bind_shader(const Context *context, CommandBuffer cmd,
                     ShaderObject shader, ShaderStage stage) noexcept;

void cmd_unbind_shaders(const Context *context, CommandBuffer cmd,
                        ShaderStage stages) noexcept;

void cmd_draw(CommandBuffer cmd, uint32_t vertexCount, uint32_t instanceCount,
              uint32_t firstVertex, uint32_t firstInstance) noexcept;

void cmd_draw_indexed(CommandBuffer cmd, uint32_t indexCount,
                      uint32_t instanceCount, uint32_t firstIndex,
                      uint32_t vertexOffset, uint32_t firstInstance) noexcept;

} // namespace strobe::rhi::vulkan
