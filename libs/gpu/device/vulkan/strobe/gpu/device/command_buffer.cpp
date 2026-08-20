#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/gpu/device/cmd_gpu_zone.hpp"
#include "strobe/gpu/device/command_buffer_handle_alloc.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/command_buffer_rendering_state.hpp"
#include "strobe/gpu/device/command_buffer_type.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include <fmt/format.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

#define CmdZoneScopedN(impl, name)                                             \
  ZoneScopedN(name);                                                           \
                                                                               \
  static constexpr tracy::SourceLocationData TracyConcat(                      \
      strobeGpuSourceLocation, TracyLine){                                     \
      name, TracyFunction, TracyFile, static_cast<uint32_t>(TracyLine), 0};    \
                                                                               \
  strobe::gpu::CmdGpuZone TracyConcat(strobeGpuZone, TracyLine){               \
      (impl) -> tracyCtx(), &TracyConcat(strobeGpuSourceLocation, TracyLine),  \
      (impl)->cmd.handle,                                                      \
      ((impl)->flags & strobe::gpu::CommandBufferFlags::reusable) == 0}

namespace {

using handle_alloc = cmd_buf_handle_allocator_ref;

void unpin_command_buffer(void *h) noexcept {
  if (h == nullptr) {
    return;
  }

  using control_block = handle_control_block<CommandBufferImpl, handle_alloc>;
  using allocator_traits = AllocatorTraits<handle_alloc>;

  auto *block = static_cast<control_block *>(h);

  if (block->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  CommandPool keepAlive = block->value.pool;

  handle_alloc alloc = std::move(block->alloc);
  std::destroy_at(block);
  allocator_traits::template deallocate<control_block>(alloc, block);
}

} // namespace

CommandBuffer::CommandBuffer(const CommandBuffer &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl, handle_alloc>(m_handle);
  }
}

CommandBuffer::CommandBuffer(CommandBuffer &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

CommandBuffer &CommandBuffer::operator=(const CommandBuffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl, handle_alloc>(o.m_handle);
  }
  unpin_command_buffer(m_handle);
  m_handle = o.m_handle;
  return *this;
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_command_buffer(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

CommandBuffer::~CommandBuffer() noexcept { unpin_command_buffer(m_handle); }

void CommandBuffer::begin() {
  ZoneScopedN("CommandBuffer::begin");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);
  impl->uninitialized = CommandBufferRenderingState::all;
  impl->required = CommandBufferRenderingState::graphics_pipeline_requirements;
  vulkan::begin_command_buffer(impl->cmd);
}

void CommandBuffer::end() {
  ZoneScopedN("CommandBuffer::end");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);
  vulkan::end_command_buffer(impl->cmd);
}

void CommandBuffer::memory_barrier(const MemoryBarrier &barrier) {
  ZoneScopedN("CommandBuffer::memory_barrier");
  auto *impl = void_handle_ptr<CommandBufferImpl, handle_alloc>(m_handle);
  impl->memory_barrier(barrier);
}

void CommandBuffer::begin_rendering(const RenderingInfo &info) noexcept {
  ZoneScopedN("CommandBuffer::begin_rendering");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->begin_rendering(info);
}

void CommandBuffer::end_rendering() noexcept {
  ZoneScopedN("CommandBuffer::end_rendering");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->end_rendering();
}

void CommandBuffer::set_viewports(span<const Viewport> viewports) noexcept {
  ZoneScopedN("CommandBuffer::set_viewports");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_viewports(viewports);
}

void CommandBuffer::set_scissors(span<const Rect> scissors) noexcept {
  ZoneScopedN("CommandBuffer::set_scissors");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_scissors(scissors);
}
void CommandBuffer::set_rasterizer_discard_enable(bool discardEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_rasterizer_discard_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (discardEnable) {
    impl->required &=
        ~CommandBufferRenderingState::post_rasterization_requirements;
  } else {
    impl->required |=
        CommandBufferRenderingState::post_rasterization_requirements;
  }
  impl->set_rasterizer_discard_enable(discardEnable);
}

void CommandBuffer::set_primitive_topology(
    PrimitiveTopology topology) noexcept {
  ZoneScopedN("CommandBuffer::set_primitive_topology");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_primitive_topology(topology);
}

void CommandBuffer::set_primitive_restart(bool restartEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_primitive_restart");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_primitive_restart(restartEnable);
}

void CommandBuffer::set_cull_mode(CullMode cullMode) noexcept {
  ZoneScopedN("CommandBuffer::set_cull_mode");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_cull_mode(cullMode);
}

void CommandBuffer::set_front_face(FrontFace frontFace) noexcept {
  ZoneScopedN("CommandBuffer::set_front_face");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_front_face(frontFace);
}

void CommandBuffer::set_depth_bias_enable(bool depthBiasEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_bias_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_bias_enable(depthBiasEnable);
}

void CommandBuffer::set_depth_test_enable(bool depthTestEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (depthTestEnable) {
    impl->required |= CommandBufferRenderingState::depth_compare_op;
  } else {
    impl->required &= ~CommandBufferRenderingState::depth_compare_op;
  }
  impl->set_depth_test_enable(depthTestEnable);
}

void CommandBuffer::set_depth_write_enable(bool depthWriteEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_write_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_write_enable(depthWriteEnable);
}

void CommandBuffer::set_depth_compare_op(CompareOp op) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_compare_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_compare_op(op);
}

void CommandBuffer::set_depth_bounds_test_enable(
    bool depthBoundTestEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_bounds_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (depthBoundTestEnable) {
    impl->required |= CommandBufferRenderingState::depth_bounds;
  } else {
    impl->required &= ~CommandBufferRenderingState::depth_bounds;
  }
  impl->set_depth_bounds_test_enable(depthBoundTestEnable);
}

void CommandBuffer::set_depth_bounds(float minDepthBound,
                                     float maxDepthBound) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_bounds");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_bounds(minDepthBound, maxDepthBound);
}

void CommandBuffer::set_stencil_test_enable(bool stencilTestEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_stencil_test_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_stencil_test_enable(stencilTestEnable);
}

void CommandBuffer::set_stencil_op(StencilFace faceMask, StencilOp failOp,
                                   StencilOp passOp, StencilOp depthFailOp,
                                   CompareOp compareOp) noexcept {
  ZoneScopedN("CommandBuffer::set_stencil_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_stencil_op(faceMask, failOp, passOp, depthFailOp, compareOp);
}

void CommandBuffer::set_stencil_compare_mask(StencilFace faceMask,
                                             uint32_t compareMask) noexcept {
  ZoneScopedN("CommandBuffer::set_stencil_compare_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_stencil_compare_mask(faceMask, compareMask);
}

void CommandBuffer::set_stencil_write_mask(StencilFace faceMask,
                                           uint32_t writeMask) noexcept {
  ZoneScopedN("CommandBuffer::set_stencil_write_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_stencil_write_mask(faceMask, writeMask);
}

void CommandBuffer::set_stencil_reference(StencilFace faceMask,
                                          uint32_t reference) noexcept {
  ZoneScopedN("CommandBuffer::set_stencil_reference");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_stencil_reference(faceMask, reference);
}

void CommandBuffer::set_blend_constants(vec4 constants) noexcept {
  ZoneScopedN("CommandBuffer::set_blend_constants");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_blend_constants(constants);
}

void CommandBuffer::set_line_width(float lineWidth) noexcept {
  ZoneScopedN("CommandBuffer::set_line_width");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_line_width(lineWidth);
}

void CommandBuffer::set_depth_bias(float depthBiasConstantFactor,
                                   float depthBiasClamp,
                                   float depthBiasSlope) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_bias");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_bias(depthBiasConstantFactor, depthBiasClamp, depthBiasSlope);
}

void CommandBuffer::set_vertex_input(
    span<const VertexBinding> bindings,
    span<const VertexAttribute> attributes) noexcept {
  ZoneScopedN("CommandBuffer::set_vertex_input");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_vertex_input(bindings, attributes);
}

void CommandBuffer::set_rasterization_samples(
    SampleCount rasterizationSamples) noexcept {
  ZoneScopedN("CommandBuffer::set_rasterization_samples");

  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);

  impl->required |= CommandBufferRenderingState::sample_mask;

  impl->set_rasterization_samples(rasterizationSamples);
}

void CommandBuffer::set_sample_mask(SampleCount samples,
                                    uint64_t mask) noexcept {
  ZoneScopedN("CommandBuffer::set_sample_mask");

  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);

  impl->required |= CommandBufferRenderingState::rasterization_samples;

  impl->set_sample_mask(samples, mask);
}

void CommandBuffer::set_alpha_to_coverage_enable(
    bool alphaToCoverageEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_alpha_to_coverage_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_alpha_to_coverage_enable(alphaToCoverageEnable);
}

void CommandBuffer::set_polygon_mode(PolygonMode polygonMode) noexcept {
  ZoneScopedN("CommandBuffer::set_polygon_mode");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_polygon_mode(polygonMode);
}

void CommandBuffer::set_depth_clamp_enable(bool depthClampEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_depth_clamp_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_depth_clamp_enable(depthClampEnable);
}

void CommandBuffer::set_logic_op_enable(bool logicOpEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_logic_op_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_logic_op_enable(logicOpEnable);
}

void CommandBuffer::set_logic_op(LogicOp logicOp) noexcept {
  ZoneScopedN("CommandBuffer::set_logic_op");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_logic_op(logicOp);
}

void CommandBuffer::set_color_blend_enable(uint32_t firstAttachment,
                                           uint32_t attachmentCount,
                                           uint32_t bitmask) noexcept {
  ZoneScopedN("CommandBuffer::set_color_blend_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_color_blend_enable(firstAttachment, attachmentCount, bitmask);
}

void CommandBuffer::set_color_blend_equation(
    uint32_t firstAttachment,
    span<const BlendEquation> colorBlendEquations) noexcept {
  ZoneScopedN("CommandBuffer::set_color_blend_equation");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_color_blend_equation(firstAttachment, colorBlendEquations);
}

void CommandBuffer::set_color_write_mask(
    uint32_t firstAttachment,
    span<const ColorComponent> colorWriteMasks) noexcept {
  ZoneScopedN("CommandBuffer::set_color_write_mask");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_color_write_mask(firstAttachment, colorWriteMasks);
}

void CommandBuffer::set_alpha_to_one_enable(bool alphaToOneEnable) noexcept {
  ZoneScopedN("CommandBuffer::set_alpha_to_one_enable");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_alpha_to_one_enable(alphaToOneEnable);
}

void CommandBuffer::set_patch_control_points(
    uint32_t patchControlPoints) noexcept {
  ZoneScopedN("CommandBuffer::set_patch_control_points");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->set_patch_control_points(patchControlPoints);
}

void CommandBuffer::bind_shader(VertexShader shader) noexcept {
  ZoneScopedN("CommandBuffer::bind_shader(VertexShader)");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::vertex_shader_requirements;
  impl->bind_shader(shader);
  impl->state.boundVertexShaders.emplace_back(std::move(shader));
}

void CommandBuffer::bind_shader(FragmentShader shader) noexcept {
  ZoneScopedN("CommandBuffer::bind_shader(FragmentShader)");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  impl->required |= CommandBufferRenderingState::fragment_shader_requirements;
  impl->bind_shader(shader);
  impl->state.boundFragmentShaders.emplace_back(std::move(shader));
}

void CommandBuffer::unbind_shaders(ShaderStage stages) noexcept {
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
  impl->unbind_shaders(stages);
}

void CommandBuffer::bind_vertex_buffer(Buffer buffer,
                                       uint64_t offset) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::bind_vertex_buffer");
  buffer.commit();
  impl->bind_vertex_buffer(buffer, offset);
}

void CommandBuffer::copy_buffer(Buffer dst, Buffer src) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::copy(Buffer)");
  dst.commit();
  src.commit();
  impl->copy_buffer(dst, src);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance) noexcept {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  CmdZoneScopedN(impl, "CommandBuffer::draw");

  if (auto missing = impl->required & impl->uninitialized; missing != 0) {
    impl->set_default_rendering_state(missing);
  }
  impl->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::draw_indexed(uint32_t indexCount, uint32_t instanceCount,
                                 uint32_t firstIndex, int32_t vertexOffset,
                                 uint32_t firstInstance) noexcept {
  ZoneScopedN("CommandBuffer::draw_indexed");
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  if (auto missing = impl->required & impl->uninitialized; missing != 0) {
    impl->set_default_rendering_state(missing);
  }
  impl->draw_indexed(indexCount, instanceCount, firstIndex, vertexOffset,
                     firstInstance);
};

} // namespace strobe::gpu
