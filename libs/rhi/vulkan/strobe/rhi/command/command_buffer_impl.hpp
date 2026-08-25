#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/command/command_buffer_rendering_state.hpp"
#include "strobe/rhi/command/command_buffer_state.hpp"
#include "strobe/rhi/command/native_command_pool.hpp"
#include "strobe/rhi/command/staging_bump_allocator.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/blas_impl.hpp"
#include "strobe/rhi/memory/buffer_binding.hpp"
#include "strobe/rhi/memory/buffer_handle_alloc.hpp"
#include "strobe/rhi/memory/buffer_impl.hpp"
#include "strobe/rhi/memory/image_view_impl.hpp"
#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"
#include "strobe/rhi/types/memory_barrier.hpp"
#include "strobe/rhi/utils/access_utils.hpp"
#include "strobe/rhi/utils/always_inline.hpp"
#include "strobe/rhi/utils/attachment_load_op_utils.hpp"
#include "strobe/rhi/utils/attachment_store_op_utils.hpp"
#include "strobe/rhi/utils/blend_factor_utils.hpp"
#include "strobe/rhi/utils/blend_op_utils.hpp"
#include "strobe/rhi/utils/clear_value_utils.hpp"
#include "strobe/rhi/utils/color_component_utils.hpp"
#include "strobe/rhi/utils/compare_op_utils.hpp"
#include "strobe/rhi/utils/cull_mode_utils.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/front_face_utils.hpp"
#include "strobe/rhi/utils/logic_op_utils.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include "strobe/rhi/utils/polygon_mode_utils.hpp"
#include "strobe/rhi/utils/primitive_topology_utils.hpp"
#include "strobe/rhi/utils/resolve_mode_utils.hpp"
#include "strobe/rhi/utils/sample_count_utils.hpp"
#include "strobe/rhi/utils/stencil_face_utils.hpp"
#include "strobe/rhi/utils/stencil_op_utils.hpp"
#include "strobe/rhi/utils/vertex_input_rate_utils.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include "strobe/rhi/vulkan/shader_object.hpp"
#include <cstdio>
#include <limits>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>
#ifdef STROBE_TRACY
#include "strobe/rhi/vulkan/tracy/profiler.hpp"
#endif

namespace strobe::rhi {

struct CommandBufferImpl {

  CommandBufferImpl(CommandPool pool, NativeCommandPool *nativePool,
                    vulkan::CommandBuffer cmd, CommandBufferFlags flags,
                    const cmd_buf_state_allocator_ref &alloc) noexcept;

  CommandBufferImpl(const CommandBufferImpl &) = delete;
  CommandBufferImpl(CommandBufferImpl &&) = delete;
  CommandBufferImpl &operator=(const CommandBufferImpl &) = delete;
  CommandBufferImpl &operator=(CommandBufferImpl &&) = delete;
  ~CommandBufferImpl() noexcept;

  CommandPool pool;
  NativeCommandPool *nativePool;
  vulkan::CommandBuffer cmd;
  CommandBufferState state;

  const CommandBufferFlags flags;
  const vulkan::PNF_Functions *pnf;

  // profiling
  // [[maybe_unused]] profiler::CommandBufferScope m_profilerScope;

  // dynamic rendering state
  uint32_t renderingColorAttachmentCount = std::numeric_limits<uint32_t>::max();
  CommandBufferRenderingState uninitialized = CommandBufferRenderingState::all;
  CommandBufferRenderingState required =
      CommandBufferRenderingState::graphics_pipeline_requirements;

  // staging
  BufferBinding alloc_staging(VkDeviceSize size, uint16_t alignment);

  StagingBumpAllocator m_stageAllocator{};
  StagingBuffer *m_stageRecycleListHead = nullptr;
  StagingBuffer *m_stageRecycleListTail = nullptr;

  // recording helpers
  STROBE_ALWAYS_INLINE void memory_barrier(const MemoryBarrier &barrier) {
    VkMemoryBarrier2 memoryBarrier{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = to_vk_pipeline_stage(barrier.srcStage),
        .srcAccessMask = to_vk_access(barrier.srcAccess),
        .dstStageMask = to_vk_pipeline_stage(barrier.dstStage),
        .dstAccessMask = to_vk_access(barrier.dstAccess),
    };
    VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = 0,
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &memoryBarrier,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 0,
        .pImageMemoryBarriers = nullptr,
    };
    {
      ZoneScopedN("vkCmdPipelineBarrier2");
      vkCmdPipelineBarrier2(cmd.handle, &dependencyInfo);
    }
  }

  STROBE_ALWAYS_INLINE void
  begin_rendering(const RenderingInfo &info) noexcept {
    uvec2 minExtent = uvec2(std::numeric_limits<unsigned int>::max());
    SmallVector<VkRenderingAttachmentInfo, 4> colorAttachments{
        info.colorAttachments.size()};
    for (uint32_t i = 0; i < info.colorAttachments.size(); ++i) {
      const auto &attachment = info.colorAttachments[i];
      colorAttachments[i] = VkRenderingAttachmentInfo{
          .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext = nullptr,
          .imageView = void_handle_ptr<ImageViewImpl>(attachment.view.m_handle)
                           ->imageView.handle,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .resolveMode = to_vk_resolve_mode(attachment.resolveMode),
          .resolveImageView = attachment.resolveView
                                  ? (void_handle_ptr<ImageViewImpl>(
                                         attachment.resolveView.m_handle)
                                         ->imageView.handle)
                                  : VK_NULL_HANDLE,
          .resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = to_vk_attachment_load_op(attachment.loadOp),
          .storeOp = to_vk_attachment_store_op(attachment.storeOp),
          .clearValue = to_vk_clear_value(attachment.clearValue),
      };
      minExtent.x() =
          std::min(minExtent.x(), attachment.view.image().extent().x());
      minExtent.y() =
          std::min(minExtent.y(), attachment.view.image().extent().y());
    }
    VkRenderingAttachmentInfo depthAttachment;
    VkRenderingAttachmentInfo *pDepthAttachment = nullptr;
    if (info.depthAttachment.has_value()) {
      const auto &attachment = info.depthAttachment.value();
      pDepthAttachment = &depthAttachment;
      depthAttachment = VkRenderingAttachmentInfo{
          .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext = nullptr,
          .imageView = void_handle_ptr<ImageViewImpl>(attachment.view.m_handle)
                           ->imageView.handle,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .resolveMode = to_vk_resolve_mode(attachment.resolveMode),
          .resolveImageView = attachment.resolveView
                                  ? (void_handle_ptr<ImageViewImpl>(
                                         attachment.resolveView.m_handle)
                                         ->imageView.handle)
                                  : VK_NULL_HANDLE,
          .resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = to_vk_attachment_load_op(attachment.loadOp),
          .storeOp = to_vk_attachment_store_op(attachment.storeOp),
          .clearValue = to_vk_clear_value(attachment.clearValue),
      };
      minExtent.x() =
          std::min(minExtent.x(), attachment.view.image().extent().x());
      minExtent.y() =
          std::min(minExtent.y(), attachment.view.image().extent().y());
    }

    VkRenderingAttachmentInfo stencilAttachment;
    VkRenderingAttachmentInfo *pStencilAttachment = nullptr;
    if (info.stencilAttachment.has_value()) {
      const auto &attachment = info.stencilAttachment.value();
      pStencilAttachment = &stencilAttachment;
      stencilAttachment = VkRenderingAttachmentInfo{
          .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext = nullptr,
          .imageView = void_handle_ptr<ImageViewImpl>(attachment.view.m_handle)
                           ->imageView.handle,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .resolveMode = to_vk_resolve_mode(attachment.resolveMode),
          .resolveImageView = attachment.resolveView
                                  ? (void_handle_ptr<ImageViewImpl>(
                                         attachment.resolveView.m_handle)
                                         ->imageView.handle)
                                  : VK_NULL_HANDLE,
          .resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = to_vk_attachment_load_op(attachment.loadOp),
          .storeOp = to_vk_attachment_store_op(attachment.storeOp),
          .clearValue = to_vk_clear_value(attachment.clearValue),
      };
      minExtent.x() =
          std::min(minExtent.x(), attachment.view.image().extent().x());
      minExtent.y() =
          std::min(minExtent.y(), attachment.view.image().extent().y());
    }

    VkRect2D renderArea = {
        .offset = {.x = info.renderArea.offset.x(),
                   .y = info.renderArea.offset.y()},
        .extent = {.width = info.renderArea.extent.x(),
                   .height = info.renderArea.extent.y()},
    };
    if (info.renderArea.offset == ivec2(0, 0) &&
        info.renderArea.extent == uvec2(0, 0)) {
      renderArea.extent = {.width = minExtent.x(), .height = minExtent.y()};
    }

    VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = renderArea,
        .layerCount = info.layerCount,
        .viewMask = info.viewMask,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
        .pColorAttachments = colorAttachments.data(),
        .pDepthAttachment = pDepthAttachment,
        .pStencilAttachment = pStencilAttachment,
    };
    renderingColorAttachmentCount = info.colorAttachments.size();

    ZoneScopedN("vkCmdBeginRendering");
    vkCmdBeginRendering(cmd.handle, &renderInfo);
  }

  STROBE_ALWAYS_INLINE void end_rendering() noexcept {
    renderingColorAttachmentCount = std::numeric_limits<uint32_t>::max();
    ZoneScopedN("vkCmdEndRendering");
    vkCmdEndRendering(cmd.handle);
  }

  STROBE_ALWAYS_INLINE void
  set_viewports(span<const Viewport> viewports) noexcept {
    SmallVector<VkViewport, 2> views{viewports.size()};
    for (uint32_t i = 0; i < viewports.size(); ++i) {
      views[i] = VkViewport{
          .x = viewports[i].position.x(),
          .y = viewports[i].position.y(),
          .width = viewports[i].extent.x(),
          .height = viewports[i].extent.y(),
          .minDepth = viewports[i].minDepth,
          .maxDepth = viewports[i].maxDepth,
      };
    }
    ZoneScopedN("vkCmdSetViewportWithCount");
    vkCmdSetViewportWithCount(cmd.handle, views.size(), views.data());
  }

  STROBE_ALWAYS_INLINE void set_scissors(span<const Rect> scissors) noexcept {
    SmallVector<VkRect2D, 2> sci{scissors.size()};
    for (uint32_t i = 0; i < scissors.size(); ++i) {
      sci[i] = VkRect2D{
          .offset = {.x = scissors[i].offset.x(), .y = scissors[i].offset.y()},
          .extent = {.width = scissors[i].extent.x(),
                     .height = scissors[i].extent.y()},
      };
    }
    ZoneScopedN("vkCmdSetScissorWithCount");
    vkCmdSetScissorWithCount(cmd.handle, sci.size(), sci.data());
  }

  STROBE_ALWAYS_INLINE void
  set_rasterizer_discard_enable(bool discardEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::rasterizer_discard_enable;
    ZoneScopedN("vkCmdSetRasterizerDiscardEnable");
    vkCmdSetRasterizerDiscardEnable(cmd.handle,
                                    static_cast<VkBool32>(discardEnable));
  }

  STROBE_ALWAYS_INLINE void
  set_primitive_topology(PrimitiveTopology topology) noexcept {
    uninitialized &= ~CommandBufferRenderingState::primitive_topology;
    VkPrimitiveTopology top = to_vk_primitive_topology(topology);
    ZoneScopedN("vkCmdSetPrimitiveTopology");
    vkCmdSetPrimitiveTopology(cmd.handle, top);
  }

  STROBE_ALWAYS_INLINE void set_primitive_restart(bool restartEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::primitive_restart_enable;
    ZoneScopedN("vkCmdSetPrimitiveRestartEnable");
    vkCmdSetPrimitiveRestartEnable(cmd.handle,
                                   static_cast<VkBool32>(restartEnable));
  }

  STROBE_ALWAYS_INLINE void set_cull_mode(CullMode cullMode) noexcept {
    uninitialized &= ~CommandBufferRenderingState::cull_mode;
    VkCullModeFlags mode = to_vk_cull_mode(cullMode);
    ZoneScopedN("vkCmdSetCullMode");
    vkCmdSetCullMode(cmd.handle, mode);
  }

  STROBE_ALWAYS_INLINE void set_front_face(FrontFace frontFace) noexcept {
    uninitialized &= ~CommandBufferRenderingState::front_face;
    VkFrontFace face = to_vk_front_face(frontFace);
    ZoneScopedN("vkCmdSetFrontFace");
    vkCmdSetFrontFace(cmd.handle, face);
  }

  STROBE_ALWAYS_INLINE void
  set_depth_bias_enable(bool depthBiasEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_bias_enable;
    ZoneScopedN("vkCmdSetDepthBiasEnable");
    vkCmdSetDepthBiasEnable(cmd.handle, static_cast<VkBool32>(depthBiasEnable));
  }

  STROBE_ALWAYS_INLINE void
  set_depth_test_enable(bool depthTestEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_test_enable;
    ZoneScopedN("vkCmdSetDepthTestEnable");
    vkCmdSetDepthTestEnable(cmd.handle, static_cast<VkBool32>(depthTestEnable));
  }

  STROBE_ALWAYS_INLINE void
  set_depth_write_enable(bool depthWriteEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_write_enable;
    ZoneScopedN("vkCmdSetDepthWriteEnable");
    vkCmdSetDepthWriteEnable(cmd.handle,
                             static_cast<VkBool32>(depthWriteEnable));
  }

  STROBE_ALWAYS_INLINE void set_depth_compare_op(CompareOp op) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_compare_op;
    VkCompareOp face = to_vk_compare_op(op);
    ZoneScopedN("vkCmdSetDepthCompareOp");
    vkCmdSetDepthCompareOp(cmd.handle, face);
  }

  STROBE_ALWAYS_INLINE void
  set_depth_bounds_test_enable(bool depthBoundTestEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_bounds_test_enable;
    ZoneScopedN("vkCmdSetDepthBoundsTestEnable");
    vkCmdSetDepthBoundsTestEnable(cmd.handle,
                                  static_cast<VkBool32>(depthBoundTestEnable));
  }

  STROBE_ALWAYS_INLINE void set_depth_bounds(float minDepthBound,
                                             float maxDepthBound) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_bounds;
    ZoneScopedN("vkCmdSetDepthBounds");
    vkCmdSetDepthBounds(cmd.handle, minDepthBound, maxDepthBound);
  }

  STROBE_ALWAYS_INLINE void
  set_stencil_test_enable(bool stencilTestEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::stencil_test_enable;
    ZoneScopedN("vkCmdSetStencilTestEnable");
    vkCmdSetStencilTestEnable(cmd.handle,
                              static_cast<VkBool32>(stencilTestEnable));
  }

  STROBE_ALWAYS_INLINE void set_stencil_op(StencilFace faceMask,
                                           StencilOp failOp, StencilOp passOp,
                                           StencilOp depthFailOp,
                                           CompareOp compareOp) noexcept {
    const VkStencilFaceFlags face = to_vk_stencil_face(faceMask);
    const VkStencilOp fail = to_vk_stencil_op(failOp);
    const VkStencilOp pass = to_vk_stencil_op(passOp);
    const VkStencilOp depthFail = to_vk_stencil_op(depthFailOp);
    const VkCompareOp cop = to_vk_compare_op(compareOp);

    ZoneScopedN("vkCmdSetStencilOp");
    vkCmdSetStencilOp(cmd.handle, face, fail, pass, depthFail, cop);
  }

  STROBE_ALWAYS_INLINE void
  set_stencil_compare_mask(StencilFace faceMask,
                           uint32_t compareMask) noexcept {
    uninitialized &= ~CommandBufferRenderingState::stencil_compare_mask;
    const VkStencilFaceFlags face = to_vk_stencil_face(faceMask);
    ZoneScopedN("vkCmdSetStencilCompareMask");
    vkCmdSetStencilCompareMask(cmd.handle, face, compareMask);
  }

  STROBE_ALWAYS_INLINE void
  set_stencil_write_mask(StencilFace faceMask, uint32_t writeMask) noexcept {
    const VkStencilFaceFlags face = to_vk_stencil_face(faceMask);
    ZoneScopedN("vkCmdSetStencilWriteMask");
    vkCmdSetStencilWriteMask(cmd.handle, face, writeMask);
  }

  STROBE_ALWAYS_INLINE void set_stencil_reference(StencilFace faceMask,
                                                  uint32_t reference) noexcept {
    const VkStencilFaceFlags face = to_vk_stencil_face(faceMask);
    ZoneScopedN("vkCmdSetStencilReference");
    vkCmdSetStencilReference(cmd.handle, face, reference);
  }

  STROBE_ALWAYS_INLINE void set_blend_constants(vec4 constants) noexcept {
    float blendConstants[4] = {
        constants.x(),
        constants.y(),
        constants.z(),
        constants.w(),
    };
    ZoneScopedN("vkCmdSetBlendConstants");
    vkCmdSetBlendConstants(cmd.handle, blendConstants);
  }

  STROBE_ALWAYS_INLINE void set_line_width(float lineWidth) noexcept {
    ZoneScopedN("vkCmdSetLineWidth");
    vkCmdSetLineWidth(cmd.handle, lineWidth);
  }

  STROBE_ALWAYS_INLINE void set_depth_bias(float depthBiasConstantFactor,
                                           float depthBiasClamp,
                                           float depthBiasSlope) noexcept {
    ZoneScopedN("vkCmdSetDepthBias");
    vkCmdSetDepthBias(cmd.handle, depthBiasConstantFactor, depthBiasClamp,
                      depthBiasSlope);
  }

  STROBE_ALWAYS_INLINE void
  set_vertex_input(span<const VertexBinding> bindings,
                   span<const VertexAttribute> attributes) noexcept {
    uninitialized &= ~CommandBufferRenderingState::vertex_input;
    static constexpr size_t SCRATCH_SIZE =
        sizeof(VkVertexInputBindingDescription2EXT) * 8 +
        sizeof(VkVertexInputAttributeDescription2EXT) * 8;
    using scratch_allocator =
        InplaceMonotonicResource<strobe::rhi::allocator, SCRATCH_SIZE>;
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
    ZoneScopedN("vkCmdSetVertexInput");
    vulkan::vk_cmd_set_vertex_input(
        pnf, cmd.handle, bindingDescriptions.size(), bindingDescriptions.data(),
        attributeDescriptions.size(), attributeDescriptions.data());
  }
  STROBE_ALWAYS_INLINE void
  set_rasterization_samples(SampleCount rasterizationSamples) noexcept {
    uninitialized &= ~CommandBufferRenderingState::rasterization_samples;
    const VkSampleCountFlagBits samples =
        to_vk_sample_count(rasterizationSamples);
    ZoneScopedN("vkCmdSetRasterizationSamples");
    vulkan::vk_cmd_set_rasterization_samples(pnf, cmd.handle, samples);
  }

  STROBE_ALWAYS_INLINE void set_sample_mask(SampleCount samples,
                                            uint64_t mask) noexcept {
    uninitialized &= ~CommandBufferRenderingState::sample_mask;
    const VkSampleCountFlagBits sampleCount = to_vk_sample_count(samples);
    const VkSampleMask masks[2] = {
        static_cast<uint32_t>(mask),
        static_cast<uint32_t>(mask >> 32),
    };
    ZoneScopedN("vkCmdSetSampleMask");
    vulkan::vk_cmd_set_sample_mask(pnf, cmd.handle, sampleCount, masks);
  }

  STROBE_ALWAYS_INLINE void
  set_alpha_to_coverage_enable(bool alphaToCoverageEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::alpha_to_coverage_enable;
    ZoneScopedN("vkCmdSetAlphaToCoverageEnable");
    vulkan::vk_cmd_set_alpha_to_coverage_enable(pnf, cmd.handle,
                                                alphaToCoverageEnable);
  }

  STROBE_ALWAYS_INLINE void set_polygon_mode(PolygonMode polygonMode) noexcept {
    uninitialized &= ~CommandBufferRenderingState::polygon_mode;
    const VkPolygonMode mode = to_vk_polygon_mode(polygonMode);
    ZoneScopedN("vkCmdSetPolygonMode");
    vulkan::vk_cmd_set_polygon_mode(pnf, cmd.handle, mode);
  }

  STROBE_ALWAYS_INLINE void
  set_depth_clamp_enable(bool depthClampEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::depth_clamp_enable;
    ZoneScopedN("vkCmdSetDepthClampEnable");
    vulkan::vk_cmd_set_depth_clamp_enable(pnf, cmd.handle, depthClampEnable);
  }

  STROBE_ALWAYS_INLINE void set_logic_op_enable(bool logicOpEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::logic_op_enable;
    ZoneScopedN("vkCmdSetLogicOpEnable");
    vulkan::vk_cmd_set_logic_op_enable(pnf, cmd.handle, logicOpEnable);
  }

  STROBE_ALWAYS_INLINE void set_logic_op(LogicOp logicOp) noexcept {
    VkLogicOp op = to_vk_logic_op(logicOp);
    ZoneScopedN("vkCmdSetLogicOp");
    vulkan::vk_cmd_set_logic_op(pnf, cmd.handle, op);
  }

  STROBE_ALWAYS_INLINE void set_color_blend_enable(uint32_t firstAttachment,
                                                   uint32_t attachmentCount,
                                                   uint32_t bitmask) noexcept {
    uninitialized &= ~CommandBufferRenderingState::color_blend_enable;
    assert(attachmentCount <= 32);
    VkBool32 enable[32]{};
    for (uint32_t i = 0; i < attachmentCount; ++i) {
      enable[i] = (bitmask & (uint32_t{1} << i)) ? VK_TRUE : VK_FALSE;
    }

    ZoneScopedN("vkCmdSetColorBlendEnable");
    vulkan::vk_cmd_set_color_blend_enable(pnf, cmd.handle, firstAttachment,
                                          attachmentCount, enable);
  }

  STROBE_ALWAYS_INLINE void set_color_blend_equation(
      uint32_t firstAttachment,
      span<const BlendEquation> colorBlendEquations) noexcept {
    SmallVector<VkColorBlendEquationEXT, 4> equations{
        colorBlendEquations.size()};
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
    ZoneScopedN("vkCmdSetColorBlendEquation");
    vulkan::vk_cmd_set_color_blend_equation(pnf, cmd.handle, firstAttachment,
                                            equations.size(), equations.data());
  }

  STROBE_ALWAYS_INLINE void
  set_color_write_mask(uint32_t firstAttachment,
                       span<const ColorComponent> colorWriteMasks) noexcept {
    uninitialized &= ~CommandBufferRenderingState::color_write_mask;
    SmallVector<VkColorComponentFlags, 4> masks(colorWriteMasks.size());
    for (uint32_t i = 0; i < masks.size(); ++i) {
      const auto &mask = colorWriteMasks[i];
      masks[i] = to_vk_color_component(mask);
    }

    ZoneScopedN("vkCmdSetColorWriteMask");
    vulkan::vk_cmd_set_color_write_mask(pnf, cmd.handle, firstAttachment,
                                        masks.size(), masks.data());
  }

  STROBE_ALWAYS_INLINE void
  set_alpha_to_one_enable(bool alphaToOneEnable) noexcept {
    uninitialized &= ~CommandBufferRenderingState::alpha_to_one_enable;
    ZoneScopedN("vkCmdSetAlphaToOneEnable");
    vulkan::vk_cmd_set_alpha_to_one_enable(
        pnf, cmd.handle, static_cast<VkBool32>(alphaToOneEnable));
  }

  STROBE_ALWAYS_INLINE void
  set_patch_control_points(uint32_t patchControlPoints) noexcept {
    ZoneScopedN("vkCmdSetPatchControlPoints");
    vulkan::vk_cmd_set_patch_control_points(pnf, cmd.handle,
                                            patchControlPoints);
  }

  STROBE_ALWAYS_INLINE void bind_shader(vulkan::ShaderObject shader,
                                        VkShaderStageFlagBits stage) noexcept {
    ZoneScopedN("vkCmdBindShaders");
    vulkan::vk_cmd_bind_shaders(pnf, cmd.handle, 1, &stage, &shader.handle);
  }

  STROBE_ALWAYS_INLINE void unbind_shaders(ShaderStage stages) noexcept {
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
    static_assert(static_cast<uint32_t>(ShaderStage::intersection) ==
                  (1u << 10));
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

    vulkan::vk_cmd_bind_shaders(pnf, cmd.handle, stageCount, vkStages, nullptr);
  }

  STROBE_ALWAYS_INLINE void bind_vertex_buffer(const Buffer &buffer,
                                               VkDeviceSize offset) {
    auto *buffer_impl =
        void_handle_ptr<BufferImpl, buffer_handle_alloc_ref>(buffer.m_handle);
    {
      ZoneScopedN("vkCmdBindVertexBuffers");
      vkCmdBindVertexBuffers(cmd.handle, 0, 1, &buffer_impl->buffer.handle,
                             &offset);
    }
  }

  STROBE_ALWAYS_INLINE void copy_buffer(const Buffer &dst, const Buffer &src) {
    auto *dst_impl =
        void_handle_ptr<BufferImpl, buffer_handle_alloc_ref>(dst.m_handle);
    auto *src_impl =
        void_handle_ptr<BufferImpl, buffer_handle_alloc_ref>(src.m_handle);
    assert(dst.size() == src.size());
    VkBufferCopy copy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = dst.size(),
    };
    {
      ZoneScopedN("vkCmdCopyBuffer");
      vkCmdCopyBuffer(cmd.handle, src_impl->buffer.handle,
                      dst_impl->buffer.handle, 1, &copy);
    }
  }

  STROBE_ALWAYS_INLINE void copy_buffer(vulkan::Buffer dst, vulkan::Buffer src,
                                        VkDeviceSize srcOffset,
                                        VkDeviceSize dstOffset,
                                        VkDeviceSize size) {
    assert(dst);
    assert(src);
    VkBufferCopy copy{
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = size,
    };
    {
      ZoneScopedN("vkCmdCopyBuffer");
      vkCmdCopyBuffer(cmd.handle, src.handle, dst.handle, 1, &copy);
    }
  }

  STROBE_ALWAYS_INLINE void update_buffer(vulkan::Buffer dst,
                                          VkDeviceSize dstOffset,
                                          VkDeviceSize dataSize,
                                          const void *pData) {
    assert(dst);
    assert(pData);
    ZoneScopedN("vkCmdUpdateBuffer");
    vkCmdUpdateBuffer(cmd.handle, dst.handle, dstOffset, dataSize, pData);
  }

  STROBE_ALWAYS_INLINE void draw(uint32_t vertexCount, uint32_t instanceCount,
                                 uint32_t firstVertex,
                                 uint32_t firstInstance) noexcept {
    ZoneScopedN("vkCmdDraw");
    vkCmdDraw(cmd.handle, vertexCount, instanceCount, firstVertex,
              firstInstance);
  }

  STROBE_ALWAYS_INLINE void
  draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
               int32_t vertexOffset, uint32_t firstInstance) noexcept {
    ZoneScopedN("vkCmdDrawIndexed");
    vkCmdDrawIndexed(cmd.handle, indexCount, instanceCount, firstIndex,
                     vertexOffset, firstInstance);
  };

  STROBE_ALWAYS_INLINE void
  build_acceleration_structure(const Blas &blas,
                               span<const BuildRangeInfo> &ranges,
                               VkDeviceSize scratch) {
    auto *blas_impl = void_handle_ptr<BlasImpl>(blas.m_handle);
    assert(blas_impl->accelerationStructure);

    assert(ranges.size() == blas_impl->geometries.size());

    SmallVector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges{
        ranges.size()};
    for (uint32_t i = 0; i < ranges.size(); ++i) {
      buildRanges[i] = VkAccelerationStructureBuildRangeInfoKHR{
          .primitiveCount = ranges[i].primitiveCount,
          .primitiveOffset = ranges[i].primitiveOffset,
          .firstVertex = ranges[i].firstVertex,
          .transformOffset = ranges[i].transformOffset,
      };
    }

    VkAccelerationStructureBuildGeometryInfoKHR info{
        .sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = blas_impl->buildFlags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .srcAccelerationStructure = VK_NULL_HANDLE,
        .dstAccelerationStructure = blas_impl->accelerationStructure.handle,
        .geometryCount = static_cast<uint32_t>(blas_impl->geometries.size()),
        .pGeometries = blas_impl->geometries.data(),
        .ppGeometries = nullptr,
        .scratchData =
            VkDeviceOrHostAddressKHR{
                .deviceAddress = scratch,
            },
    };

    const VkAccelerationStructureBuildRangeInfoKHR *pRanges =
        buildRanges.data();

    ZoneScopedN("vkCmdBuildAccelerationStructures");
    vulkan::vk_cmd_build_acceleration_structures(pnf, cmd.handle, 1, &info,
                                                 &pRanges);
  }

  void
  set_default_rendering_state(CommandBufferRenderingState states) noexcept {
    auto mask =
        static_cast<std::underlying_type_t<CommandBufferRenderingState>>(
            states);
    while (mask != 0) {
      const uint32_t index = static_cast<uint32_t>(std::countr_zero(mask));
      const auto cmd = static_cast<CommandBufferRenderingCmd>(index);
      switch (cmd) {
      case CommandBufferRenderingCmd::rasterizer_discard_enable:
        set_rasterizer_discard_enable(false);
        break;
      case CommandBufferRenderingCmd::primitive_topology:
        set_primitive_topology(PrimitiveTopology::triangle_list);
        break;
      case CommandBufferRenderingCmd::primitive_restart_enable:
        set_primitive_restart(false);
        break;
      case CommandBufferRenderingCmd::cull_mode:
        set_cull_mode(CullMode::none);
        break;
      case CommandBufferRenderingCmd::front_face:
        set_front_face(FrontFace::counter_clockwise);
        break;
      case CommandBufferRenderingCmd::depth_bias_enable:
        set_depth_bias_enable(false);
        break;
      case CommandBufferRenderingCmd::depth_test_enable:
        set_depth_test_enable(false);
        break;
      case CommandBufferRenderingCmd::depth_write_enable:
        set_depth_write_enable(false);
        break;
      case CommandBufferRenderingCmd::depth_compare_op:
        set_depth_compare_op(CompareOp::less);
        break;
      case CommandBufferRenderingCmd::depth_bounds_test_enable:
        set_depth_bounds_test_enable(false);
        break;
      case CommandBufferRenderingCmd::depth_bounds:
        set_depth_bounds(0.0f, 1.0f);
        break;
      case CommandBufferRenderingCmd::stencil_test_enable:
        set_stencil_test_enable(false);
        break;
      case CommandBufferRenderingCmd::stencil_compare_mask:
        set_stencil_compare_mask(StencilFace::front_and_back, 0xffffffffu);
        break;
      case CommandBufferRenderingCmd::vertex_input:
        set_vertex_input({}, {});
        break;
      case CommandBufferRenderingCmd::rasterization_samples: {
        const bool sampleMaskUninitialized =
            (uninitialized & CommandBufferRenderingState::sample_mask) != 0;
        set_rasterization_samples(SampleCount::x1);
        if (sampleMaskUninitialized) {
          set_sample_mask(SampleCount::x1,
                          std::numeric_limits<uint64_t>::max());
        }
        break;
      }
      case CommandBufferRenderingCmd::sample_mask:
        break; // just let validation catch me
      case CommandBufferRenderingCmd::alpha_to_coverage_enable:
        set_alpha_to_coverage_enable(false);
        break;
      case CommandBufferRenderingCmd::polygon_mode:
        set_polygon_mode(PolygonMode::fill);
        break;
      case CommandBufferRenderingCmd::depth_clamp_enable:
        set_depth_clamp_enable(false);
        break;
      case CommandBufferRenderingCmd::logic_op_enable:
        set_logic_op_enable(false);
        break;
      case CommandBufferRenderingCmd::color_blend_enable:
        assert(renderingColorAttachmentCount !=
                   std::numeric_limits<uint32_t>::max() &&
               "likely a draw-call outside of rendering");
        set_color_blend_enable(0, renderingColorAttachmentCount, 0);
        break;
      case CommandBufferRenderingCmd::color_write_mask: {
        assert(renderingColorAttachmentCount !=
                   std::numeric_limits<uint32_t>::max() &&
               "likely a draw-call outside of rendering");
        SmallVector<ColorComponent, 4> components{renderingColorAttachmentCount,
                                                  ColorComponent::rgba};
        set_color_write_mask(0, components);
        break;
      }
      case CommandBufferRenderingCmd::alpha_to_one_enable:
        set_alpha_to_one_enable(false);
        break;
      case CommandBufferRenderingCmd::count:
        std::unreachable();
      }
      // Remove lowest set bit.
      mask &= mask - 1;
    }
  }
};

} // namespace strobe::rhi
