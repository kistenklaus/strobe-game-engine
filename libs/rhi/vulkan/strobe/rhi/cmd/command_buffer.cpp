#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/bvh/bvh_impl.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_buffer_rendering_state.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"
#include "strobe/rhi/img/image_view_impl.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/utils/attachment_load_op_utils.hpp"
#include "strobe/rhi/utils/attachment_store_op_utils.hpp"
#include "strobe/rhi/utils/clear_value_utils.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/resolve_mode_utils.hpp"
#include "strobe/rhi/vulkan/cmd/barrier.hpp"
#include "strobe/rhi/vulkan/cmd/rendering.hpp"
#include "strobe/rhi/vulkan/cmd/transfer.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>

#ifdef STROBE_RHI_TRACE_DEVICE
#include "strobe/rhi/vulkan/tracy/profiler.hpp"
#endif

namespace strobe::rhi {

#ifdef STROBE_RHI_TRACE_DEVICE
#define CmdZoneScopedN(impl, name)                                             \
  ZoneScopedN(name);                                                           \
  static constexpr tracy::SourceLocationData TracyConcat(                      \
      strobeRhiSourceLocation, TracyLine){                                     \
      name, TracyFunction, TracyFile, static_cast<uint32_t>(TracyLine), 0};    \
  strobe::rhi::profiler::CmdScope TracyConcat(strobeRhiZone, TracyLine){       \
      &(impl) -> m_profilerScope,                                              \
      &TracyConcat(strobeRhiSourceLocation, TracyLine), (impl)->cmd,           \
      ((impl)->flags & strobe::rhi::CommandBufferFlags::reusable) == 0}
#else
#define CmdZoneScopedN(impl, name) ZoneScopedN(name);
#endif

CommandBuffer::CommandBuffer(const CommandBuffer &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl>(m_handle);
  }
}

CommandBuffer::CommandBuffer(CommandBuffer &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

CommandBuffer &CommandBuffer::operator=(const CommandBuffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl>(o.m_handle);
  }
  unpin_void_handle<CommandBufferImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<CommandBufferImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

CommandBuffer::~CommandBuffer() noexcept {
  unpin_void_handle<CommandBufferImpl>(m_handle);
}

void CommandBuffer::begin() {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::begin");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->uninitialized = CommandBufferRenderingState::all;
  impl->required = CommandBufferRenderingState::graphics_pipeline_requirements;
  vulkan::begin_command_buffer(impl->cmd);
}

void CommandBuffer::end() {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::end");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::end_command_buffer(impl->cmd);
}

void CommandBuffer::memory_barrier(const MemoryBarrier &barrier) {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::memory_barrier");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_memory_barrier(impl->cmd, barrier, 0);
}

void CommandBuffer::memory_barrier(AccessScope src, AccessScope dst) {
  assert(m_handle);
  memory_barrier(MemoryBarrier{
      .srcStage = src.stage,
      .srcAccess = src.access,
      .dstStage = dst.stage,
      .dstAccess = dst.access,
  });
}
void CommandBuffer::transition_image(const Image &image, ImageLayout src,
                                     ImageLayout dst) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::transition_image");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  auto *img_impl = object_handle_ptr<ImageImpl>(image);
  vulkan::cmd_transition_image(
      impl->cmd, img_impl->image,
      ImageSubresourceRange{
          .aspect = format_image_aspects(image.format()),
          .baseMipLevel = 0,
          .levelCount = image.mip_levels(),
          .baseArrayLayer = 0,
          .layerCount = image.arrayLayers(),
      },
      src, dst);
}

void CommandBuffer::begin_rendering(const RenderingInfo &info) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::begin_rendering");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);

  SmallVector<VkRenderingAttachmentInfo, 4> colorAttachments{
      info.colorAttachments.size()};
  uvec2 minExtent = uvec2(std::numeric_limits<unsigned int>::max());
  for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
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
  }

  std::optional<VkRenderingAttachmentInfo> depthAttachment{};
  if (info.depthAttachment.has_value()) {
    const auto &attachment = info.depthAttachment.value();
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

  std::optional<VkRenderingAttachmentInfo> stencilAttachment{};
  if (info.stencilAttachment.has_value()) {
    const auto &attachment = info.stencilAttachment.value();
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

  Rect renderArea = info.renderArea;
  if (renderArea.extent == uvec2(0, 0)) {
    renderArea.extent = minExtent;
  }

  vulkan::cmd_begin_rendering(impl->cmd,
                              {
                                  .renderArea = renderArea,
                                  .layerCount = info.layerCount,
                                  .viewMask = info.viewMask,
                                  .colorAttachments = colorAttachments,
                                  .depthAttachment = depthAttachment,
                                  .stencilAttachment = stencilAttachment,
                              });
  impl->renderingColorAttachmentCount = info.colorAttachments.size();
}

void CommandBuffer::end_rendering() noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::end_rendering");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_end_rendering(impl->cmd);
  impl->renderingColorAttachmentCount = std::numeric_limits<uint32_t>::max();
}

void CommandBuffer::set_viewports(span<const Viewport> viewports) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_viewports");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_viewports(impl->cmd, viewports);
}

void CommandBuffer::set_scissors(span<const Rect> scissors) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_scissors");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_scissors(impl->cmd, scissors);
}

void CommandBuffer::set_rasterizer_discard_enable(bool discardEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_rasterizer_discard_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (discardEnable) {
    impl->required &=
        ~CommandBufferRenderingState::post_rasterization_requirements;
  } else {
    impl->required |=
        CommandBufferRenderingState::post_rasterization_requirements;
  }
  vulkan::cmd_set_rasterizer_discard_enable(impl->cmd, discardEnable);
  impl->uninitialized &=
      ~CommandBufferRenderingState::rasterizer_discard_enable;
}

void CommandBuffer::set_primitive_topology(
    PrimitiveTopology topology) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_primitive_topology");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_primitive_topology(impl->cmd, topology);
  impl->uninitialized &= ~CommandBufferRenderingState::primitive_topology;
}

void CommandBuffer::set_primitive_restart(bool restartEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_primitive_restart");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_primitive_restart(impl->cmd, restartEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::primitive_restart_enable;
}

void CommandBuffer::set_cull_mode(CullMode cullMode) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_cull_mode");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_cull_mode(impl->cmd, cullMode);
  impl->uninitialized &= ~CommandBufferRenderingState::cull_mode;
}

void CommandBuffer::set_front_face(FrontFace frontFace) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_front_face");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_front_face(impl->cmd, frontFace);
  impl->uninitialized &= ~CommandBufferRenderingState::front_face;
}

void CommandBuffer::set_depth_bias_enable(bool depthBiasEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_bias_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_bias_enable(impl->cmd, depthBiasEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_bias_enable;
}

void CommandBuffer::set_depth_test_enable(bool depthTestEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (depthTestEnable) {
    impl->required |= CommandBufferRenderingState::depth_compare_op;
  } else {
    impl->required &= ~CommandBufferRenderingState::depth_compare_op;
  }
  vulkan::cmd_set_depth_test_enable(impl->cmd, depthTestEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_test_enable;
}

void CommandBuffer::set_depth_write_enable(bool depthWriteEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_write_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_write_enable(impl->cmd, depthWriteEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_write_enable;
}

void CommandBuffer::set_depth_compare_op(CompareOp op) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_compare_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_compare_op(impl->cmd, op);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_compare_op;
}

void CommandBuffer::set_depth_bounds_test_enable(
    bool depthBoundsTestEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_bounds_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (depthBoundsTestEnable) {
    impl->required |= CommandBufferRenderingState::depth_bounds;
  } else {
    impl->required &= ~CommandBufferRenderingState::depth_bounds;
  }
  vulkan::cmd_set_depth_bounds_test_enable(impl->cmd, depthBoundsTestEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_bounds_test_enable;
}

void CommandBuffer::set_depth_bounds(float minDepthBound,
                                     float maxDepthBound) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_bounds");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_bounds(impl->cmd, minDepthBound, maxDepthBound);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_bounds;
}

void CommandBuffer::set_stencil_test_enable(bool stencilTestEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_stencil_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_stencil_test_enable(impl->cmd, stencilTestEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::stencil_test_enable;
}

void CommandBuffer::set_stencil_op(StencilFace faceMask, StencilOp failOp,
                                   StencilOp passOp, StencilOp depthFailOp,
                                   CompareOp compareOp) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_stencil_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_stencil_op(impl->cmd, faceMask, failOp, passOp, depthFailOp,
                             compareOp);
}

void CommandBuffer::set_stencil_compare_mask(StencilFace faceMask,
                                             uint32_t compareMask) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_stencil_compare_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_stencil_compare_mask(impl->cmd, faceMask, compareMask);
  impl->uninitialized &= ~CommandBufferRenderingState::stencil_compare_mask;
}

void CommandBuffer::set_stencil_write_mask(StencilFace faceMask,
                                           uint32_t writeMask) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_stencil_write_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_stencil_write_mask(impl->cmd, faceMask, writeMask);
}

void CommandBuffer::set_stencil_reference(StencilFace faceMask,
                                          uint32_t reference) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_stencil_reference");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_stencil_reference(impl->cmd, faceMask, reference);
}

void CommandBuffer::set_blend_constants(vec4 constants) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_blend_constants");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_blend_constants(impl->cmd, constants);
}

void CommandBuffer::set_line_width(float lineWidth) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_line_width");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_line_width(impl->cmd, lineWidth);
}

void CommandBuffer::set_depth_bias(float depthBiasConstantFactor,
                                   float depthBiasClamp,
                                   float depthBiasSlope) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_bias");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_bias(impl->cmd, depthBiasConstantFactor, depthBiasClamp,
                             depthBiasSlope);
}

void CommandBuffer::set_vertex_input(
    span<const VertexBinding> bindings,
    span<const VertexAttribute> attributes) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_vertex_input");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_vertex_input(impl->ctx, impl->cmd, bindings, attributes);
  impl->uninitialized &= ~CommandBufferRenderingState::vertex_input;
}

void CommandBuffer::set_rasterization_samples(
    SampleCount rasterizationSamples) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_rasterization_samples");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::sample_mask;
  vulkan::cmd_set_rasterizer_samples(impl->ctx, impl->cmd,
                                     rasterizationSamples);
  impl->uninitialized &= ~CommandBufferRenderingState::rasterization_samples;
}

void CommandBuffer::set_sample_mask(SampleCount samples,
                                    uint64_t mask) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_sample_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::rasterization_samples;
  vulkan::cmd_set_sample_mask(impl->ctx, impl->cmd, samples, mask);
  impl->uninitialized &= ~CommandBufferRenderingState::sample_mask;
}

void CommandBuffer::set_alpha_to_coverage_enable(
    bool alphaToCoverageEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_alpha_to_coverage_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_alpha_to_coverage_enable(impl->ctx, impl->cmd,
                                           alphaToCoverageEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::alpha_to_coverage_enable;
}

void CommandBuffer::set_polygon_mode(PolygonMode polygonMode) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_polygon_mode");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_polygon_mode(impl->ctx, impl->cmd, polygonMode);
  impl->uninitialized &= ~CommandBufferRenderingState::polygon_mode;
}

void CommandBuffer::set_depth_clamp_enable(bool depthClampEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_depth_clamp_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_depth_clamp_enable(impl->ctx, impl->cmd, depthClampEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::depth_clamp_enable;
}

void CommandBuffer::set_logic_op_enable(bool logicOpEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_logic_op_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_logic_op_enable(impl->ctx, impl->cmd, logicOpEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::logic_op_enable;
}

void CommandBuffer::set_logic_op(LogicOp logicOp) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_logic_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_logic_op(impl->ctx, impl->cmd, logicOp);
}

void CommandBuffer::set_color_blend_enable(uint32_t firstAttachment,
                                           uint32_t attachmentCount,
                                           uint32_t bitmask) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_color_blend_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_color_blend_enable(impl->ctx, impl->cmd, firstAttachment,
                                     attachmentCount, bitmask);
  impl->uninitialized &= ~CommandBufferRenderingState::color_blend_enable;
}

void CommandBuffer::set_color_blend_equation(
    uint32_t firstAttachment,
    span<const BlendEquation> colorBlendEquations) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_color_blend_equation");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_color_blend_equation(impl->ctx, impl->cmd, firstAttachment,
                                       colorBlendEquations);
}

void CommandBuffer::set_color_write_mask(
    uint32_t firstAttachment,
    span<const ColorComponent> colorWriteMasks) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_color_write_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_color_write_mask(impl->ctx, impl->cmd, firstAttachment,
                                   colorWriteMasks);
  impl->uninitialized &= ~CommandBufferRenderingState::color_write_mask;
}

void CommandBuffer::set_alpha_to_one_enable(bool alphaToOneEnable) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_alpha_to_one_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_alpha_to_one_enable(impl->ctx, impl->cmd, alphaToOneEnable);
  impl->uninitialized &= ~CommandBufferRenderingState::alpha_to_one_enable;
}

void CommandBuffer::set_patch_control_points(
    uint32_t patchControlPoints) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::set_patch_control_points");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::cmd_set_patch_control_points(impl->ctx, impl->cmd,
                                       patchControlPoints);
}

void CommandBuffer::bind_shader(const VertexShader &shader) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::bind_shader(VertexShader)");
  assert(shader);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::vertex_shader_requirements;
  vulkan::cmd_bind_shader(impl->ctx, impl->cmd,
                          object_handle_ptr<ShaderObjectImpl>(shader)->shader,
                          VK_SHADER_STAGE_VERTEX_BIT);
  impl->state.retain(shader);
}

void CommandBuffer::bind_shader(const FragmentShader &shader) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::bind_shader(FragmentShader)");
  assert(shader);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::fragment_shader_requirements;
  vulkan::cmd_bind_shader(impl->ctx, impl->cmd,
                          object_handle_ptr<ShaderObjectImpl>(shader)->shader,
                          VK_SHADER_STAGE_FRAGMENT_BIT);
  impl->state.retain(shader);
}

void CommandBuffer::bind_shader(const ComputeShader &shader) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::bind_shader(ComputeShader)");
  assert(shader);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::fragment_shader_requirements;
  vulkan::cmd_bind_shader(impl->ctx, impl->cmd,
                          object_handle_ptr<ShaderObjectImpl>(shader)->shader,
                          VK_SHADER_STAGE_COMPUTE_BIT);
  impl->state.retain(shader);
}

void CommandBuffer::unbind_shaders(ShaderStage stages) noexcept {
  assert(m_handle);
  ZoneScopedN("CommandBuffer::unbind_shaders");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CommandBufferRenderingState unrequired = CommandBufferRenderingState::none;
  if ((stages & ShaderStage::vertex) != 0) {
    unrequired |= CommandBufferRenderingState::vertex_shader_requirements;
  }
  if ((stages & ShaderStage::fragment) != 0) {
    unrequired |= CommandBufferRenderingState::fragment_shader_requirements;
  }
  impl->required &= ~unrequired;
  vulkan::cmd_unbind_shaders(impl->ctx, impl->cmd, stages);
}

void CommandBuffer::bind_vertex_buffer(const Buffer &buffer,
                                       uint64_t offset) noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  ZoneScopedN("CommandBuffer::bind_vertex_buffer");
  auto *buf_impl = object_handle_ptr<BufferImpl>(buffer);
  buf_impl->commit();
  vulkan::cmd_bind_vertex_buffer(
      impl->cmd, {.buffer = buf_impl->buffer, .offset = offset});
  impl->state.retain(buffer);
}

void CommandBuffer::copy_buffer(BufferOffset dst, BufferOffset src,
                                uint64_t size) noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::copy(Buffer)");
  auto *dst_impl = object_handle_ptr<BufferImpl>(dst.buffer);
  auto *src_impl = object_handle_ptr<BufferImpl>(src.buffer);
  assert(dst_impl->size > dst.offset);
  assert(src_impl->size > src.offset);
  dst_impl->commit();
  src_impl->commit();
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = std::min(dst_impl->size - dst.offset, src_impl->size - src.offset);
  }
  vulkan::cmd_copy_buffer(
      impl->cmd, {.buffer = dst_impl->buffer, .offset = dst.offset},
      {.buffer = src_impl->buffer, .offset = src.offset}, size);
  impl->state.retain(dst.buffer);
  impl->state.retain(src.buffer);
}

void CommandBuffer::update(BufferOffset dst, const void *src,
                           uint64_t size) noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::upload(Buffer)");
  if (size == 0) {
    return;
  }
  assert(src);
  assert(dst.buffer);
  auto *dst_impl = object_handle_ptr<BufferImpl>(dst.buffer);
  dst_impl->commit();
  assert(dst.offset <= dst_impl->size);
  assert(size <= dst_impl->size - dst.offset);

  auto *mem_impl =
      object_handle_ptr<MemoryAllocationImpl>(dst_impl->allocation);
  if (mem_impl->memoryUsage == MemoryUsage::mapped ||
      mem_impl->memoryUsage == MemoryUsage::mapped_write_sequential ||
      mem_impl->memoryUsage == MemoryUsage::mapped_incoherent) {
    std::memcpy(static_cast<std::byte *>(dst.buffer.ptr()) + dst.offset, src,
                size);
    if (mem_impl->memoryUsage == MemoryUsage::mapped_incoherent) {
      mem_impl->flush();
    }
  } else {
    constexpr size_t MAX_CMD_UPDATE = 128;
    if (size <= MAX_CMD_UPDATE) {
      vulkan::cmd_update_buffer(
          impl->cmd, {.buffer = dst_impl->buffer, .offset = dst.offset}, src,
          size);
    } else {
      impl->staged_upload(
          [&](vulkan::CommandBuffer cmd, StageBuffer stage) {
            {
              ZoneScopedN("memcpy");
              std::memcpy(stage.ptr, src, size);
            }
            vulkan::cmd_copy_buffer(
                cmd, {.buffer = dst_impl->buffer, .offset = dst.offset},
                stage.buffer, size);
          },
          size, 1);
    }
    impl->state.retain(dst.buffer);
  }
};

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::draw");

  if (auto missing = impl->required & impl->uninitialized; missing != 0) {
    impl->set_default_rendering_state(missing);
  }
  impl->flush_pc();
  vulkan::cmd_draw(impl->cmd, vertexCount, instanceCount, firstVertex,
                   firstInstance);
}

void CommandBuffer::draw_indexed(uint32_t indexCount, uint32_t instanceCount,
                                 uint32_t firstIndex, int32_t vertexOffset,
                                 uint32_t firstInstance) noexcept {
  ZoneScopedN("CommandBuffer::draw_indexed");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (auto missing = impl->required & impl->uninitialized; missing != 0) {
    impl->set_default_rendering_state(missing);
  }
  impl->flush_pc();
  vulkan::cmd_draw_indexed(impl->cmd, indexCount, instanceCount, firstIndex,
                           vertexOffset, firstInstance);
}

void CommandBuffer::build(
    const Blas &blas,
    span<const TriangleGeometryData> triangleGeometries) noexcept {
  ZoneScopedN("CommandBuffer::build(Blas, {TriangleGeometryData})");
  // Kind of a special case because the the construction is partially
  // cached. makes the code a bit more uggly =^(.
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);

  auto *blas_impl = object_handle_ptr<BvhImpl>(blas);
  auto blasLck = blas_impl->lockBuildInfo();
  auto [buildInfo, buildRange] = blas_impl->buildInfo();

  buildInfo->srcAccelerationStructure = VK_NULL_HANDLE;
  buildInfo->dstAccelerationStructure = blas_impl->accelerationStructure.handle;
  assert(buildInfo->geometryCount == triangleGeometries.size());
  auto *geometries =
      const_cast<VkAccelerationStructureGeometryKHR *>(buildInfo->pGeometries);

  for (uint32_t i = 0; i < triangleGeometries.size(); ++i) {
    const auto &triangleGeometry = triangleGeometries[i];
    assert(buildInfo->pGeometries[i].geometryType ==
           VK_GEOMETRY_TYPE_TRIANGLES_KHR);
    auto *position_impl =
        object_handle_ptr<BufferImpl>(triangleGeometry.positions.buffer);
    assert(position_impl->is_bound());
    impl->state.retain(triangleGeometry.positions.buffer);
    geometries[i].geometry.triangles.vertexData.deviceAddress =
        position_impl->address + triangleGeometry.positions.offset;
    geometries[i].geometry.triangles.vertexStride =
        triangleGeometry.positions.stride;
    buildRange[i].primitiveCount = triangleGeometry.triangleCount;
    if (geometries[i].geometry.triangles.indexType != VK_INDEX_TYPE_NONE_KHR) {
      assert(triangleGeometry.indices.has_value());
      impl->state.retain(triangleGeometry.indices->buffer);
      auto *index_impl =
          object_handle_ptr<BufferImpl>(triangleGeometry.indices->buffer);
      assert(index_impl->is_bound());
      geometries[i].geometry.triangles.indexData.deviceAddress =
          index_impl->address + triangleGeometry.indices->offset;
      buildRange[i].firstVertex = triangleGeometry.indices->vertexOffset;
    }
    if (geometries[i].geometry.triangles.transformData.hostAddress != nullptr) {
      assert(triangleGeometry.transform.has_value());
      impl->state.retain(triangleGeometry.transform->buffer);
      auto *transform_impl =
          object_handle_ptr<BufferImpl>(triangleGeometry.transform->buffer);
      assert(transform_impl->is_bound());
      geometries[i].geometry.triangles.transformData.deviceAddress =
          transform_impl->address + triangleGeometry.transform->offset;
    }
  }
  Buffer scratch = blas_impl->scratchBuffer.scratch();
  impl->state.retain(scratch);
  auto *scratch_impl = object_handle_ptr<BufferImpl>(scratch);
  scratch_impl->commit();
  buildInfo->scratchData.deviceAddress = scratch_impl->address;

  vulkan::vk_cmd_build_acceleration_structures(
      impl->ctx->pnf(), impl->cmd.handle, 1, buildInfo, &buildRange);
}

void CommandBuffer::build(
    const Blas &blas, span<const AabbGeometryData> aabbGeometries) noexcept {
  ZoneScopedN("CommandBuffer::build(Blas, {AabbGeometryData})");
  // Kind of a special case because the the construction is partially
  // cached. makes the code a bit more uggly =^(.
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);

  auto *blas_impl = object_handle_ptr<BvhImpl>(blas);
  auto blasLck = blas_impl->lockBuildInfo();
  auto [buildInfo, buildRange] = blas_impl->buildInfo();

  buildInfo->srcAccelerationStructure = VK_NULL_HANDLE;
  buildInfo->dstAccelerationStructure = blas_impl->accelerationStructure.handle;
  assert(buildInfo->geometryCount == aabbGeometries.size());
  auto *geometries =
      const_cast<VkAccelerationStructureGeometryKHR *>(buildInfo->pGeometries);

  for (uint32_t i = 0; i < aabbGeometries.size(); ++i) {
    const auto &aabbGeometry = aabbGeometries[i];
    assert(geometries[i].geometryType == VK_GEOMETRY_TYPE_AABBS_KHR);

    geometries[i].geometry.aabbs.stride = aabbGeometry.stride;
    auto *buf_impl = object_handle_ptr<BufferImpl>(aabbGeometry.buffer);
    assert(buf_impl->is_bound());
    geometries[i].geometry.aabbs.data.deviceAddress =
        buf_impl->address + aabbGeometry.offset;
    buildRange[i].primitiveCount = aabbGeometry.count;
  }
  Buffer scratch = blas_impl->scratchBuffer.scratch();
  impl->state.retain(scratch);
  auto *scratch_impl = object_handle_ptr<BufferImpl>(scratch);
  scratch_impl->commit();
  buildInfo->scratchData.deviceAddress = scratch_impl->address;

  vulkan::vk_cmd_build_acceleration_structures(
      impl->ctx->pnf(), impl->cmd.handle, 1, buildInfo, &buildRange);
}

void CommandBuffer::build([[maybe_unused]] const Tlas &blas,
                          [[maybe_unused]] BufferOffset instanceBuffer,
                          [[maybe_unused]] uint32_t count) noexcept {
  std::unreachable();
}

void CommandBuffer::push(uint32_t offset, void *data, uint32_t size) noexcept {
  ZoneScopedN("CommandBuffer::push(data)");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  std::byte *dst = &impl->pushData[offset];
  std::memcpy(dst, data, size);
  impl->pushDirtyBegin = std::min(impl->pushDirtyBegin, offset);
  impl->pushDirtyEnd = std::max(impl->pushDirtyEnd, offset + size);
}

void CommandBuffer::push(uint32_t offset,
                         const BufferDescriptor &descriptor) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  auto *desc = object_handle_ptr<BufferDescriptorImpl>(descriptor);
  push(offset, &desc->index, sizeof(uint32_t));
  impl->state.retain(descriptor);
  impl->bind_resource_heap(desc->heap, desc->ready);
  impl->dma_ready &= desc->ready;
}

void CommandBuffer::push(uint32_t offset,
                         const BufferDescriptorArray &descriptor) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  auto *desc = object_handle_ptr<BufferDescriptorArrayImpl>(descriptor);
  push(offset, &desc->index, sizeof(uint32_t));
  impl->state.retain(descriptor);
  impl->bind_resource_heap(desc->heap, desc->ready);
  impl->dma_ready &= desc->ready;
}

} // namespace strobe::rhi
