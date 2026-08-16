#pragma once

#include "strobe/gpu/device/barrier.hpp"
#include "strobe/gpu/device/blend_equation.hpp"
#include "strobe/gpu/device/color_component.hpp"
#include "strobe/gpu/device/compare_op.hpp"
#include "strobe/gpu/device/cull_mode.hpp"
#include "strobe/gpu/device/fragment_shader.hpp"
#include "strobe/gpu/device/front_face.hpp"
#include "strobe/gpu/device/logic_op.hpp"
#include "strobe/gpu/device/polygon_mode.hpp"
#include "strobe/gpu/device/primitive_topology.hpp"
#include "strobe/gpu/device/rect.hpp"
#include "strobe/gpu/device/rendering_info.hpp"
#include "strobe/gpu/device/sample_count.hpp"
#include "strobe/gpu/device/shader_stage.hpp"
#include "strobe/gpu/device/stencil_face.hpp"
#include "strobe/gpu/device/stencil_op.hpp"
#include "strobe/gpu/device/vertex_attribute.hpp"
#include "strobe/gpu/device/vertex_binding.hpp"
#include "strobe/gpu/device/vertex_shader.hpp"
#include "strobe/gpu/device/viewport.hpp"
#include <limits>

namespace strobe::gpu {

class CommandBuffer {
  friend struct CommandPoolImpl;
  friend class CommandPool;
  friend struct SwapchainImpl;
  friend class Queue;

public:
  CommandBuffer() noexcept : m_handle(nullptr) {}
  CommandBuffer(const CommandBuffer &) noexcept;
  CommandBuffer(CommandBuffer &&) noexcept;
  CommandBuffer &operator=(const CommandBuffer &) noexcept;
  CommandBuffer &operator=(CommandBuffer &&) noexcept;
  ~CommandBuffer() noexcept;

  // pool must be externally synchronized
  void begin();
  // pool must be externally synchronized
  void end();

  void barrier(const Barrier &barrier);

  // ============= rendering ===============
  void begin_rendering(const RenderingInfo &info) noexcept;
  void end_rendering() noexcept;

  void set_viewports(span<const Viewport> viewports) noexcept;
  void set_scissors(span<const Rect> scissors) noexcept;
  void set_rasterizer_discard_enable(bool discardEnable) noexcept;

  void set_primitive_topology(PrimitiveTopology topology) noexcept;
  void set_primitive_restart(bool restartEnable) noexcept;

  void set_cull_mode(CullMode cullMode) noexcept;
  void set_front_face(FrontFace frontFace) noexcept;

  void set_depth_bias_enable(bool depthBiasEnable) noexcept;
  void set_depth_test_enable(bool depthTestEnable) noexcept;
  void set_depth_write_enable(bool depthWriteEnable) noexcept;
  void set_depth_compare_op(CompareOp op) noexcept;
  void set_depth_bounds_test_enable(bool depthBoundTestEnable) noexcept;
  void set_depth_bounds(float minDepthBound, float maxDepthBound) noexcept;

  void set_stencil_test_enable(bool stencilTestEnable) noexcept;
  void set_stencil_op(StencilFace faceMask, StencilOp failOp, StencilOp passOp,
                      StencilOp depthFail, CompareOp compareOp) noexcept;
  void set_stencil_compare_mask(StencilFace faceMask,
                                uint32_t compareMask) noexcept;
  void set_stencil_write_mask(StencilFace faceMask,
                              uint32_t writeMask) noexcept;
  void set_stencil_reference(StencilFace faceMask, uint32_t reference) noexcept;
  void set_blend_constants(vec4 constants) noexcept;
  void set_line_width(float lineWidth) noexcept;
  void set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp,
                      float depthBiasSlope) noexcept;

  void set_vertex_input(span<const VertexBinding> bindings,
                        span<const VertexAttribute> attributes) noexcept;

  void set_rasterization_samples(SampleCount rasterizationSamples) noexcept;
  // check if that interface makes sense
  void set_sample_mask(
      SampleCount samples,
      uint64_t mask = std::numeric_limits<uint64_t>::max()) noexcept;
  void set_alpha_to_coverage_enable(bool alphaToCoverageEnable) noexcept;
  void set_polygon_mode(PolygonMode polygonMode) noexcept;
  void set_depth_clamp_enable(bool depthClampEnable) noexcept;
  void set_logic_op_enable(bool logicOpEnable) noexcept;
  void set_logic_op(LogicOp logicOp) noexcept;
  void set_color_blend_enable(uint32_t firstAttachment,
                              uint32_t attachmentCount,
                              uint32_t bitmask) noexcept;

  void set_color_blend_equation(
      uint32_t firstAttachment,
      span<const BlendEquation> colorBlendEquations) noexcept;

  void
  set_color_write_mask(uint32_t firstAttachment,
                       span<const ColorComponent> colorWriteMasks) noexcept;

  void set_alpha_to_one_enable(bool alphaToOneEnable) noexcept;
  void set_patch_control_points(uint32_t patchControlPoints) noexcept;

  // utility overloads.
  inline void set_color_blend_enable(uint32_t firstAttachment,
                                     uint32_t attachmentCount,
                                     bool enable) noexcept {
    set_color_blend_enable(firstAttachment, attachmentCount,
                           enable ? std::numeric_limits<uint32_t>::max() : 0);
  }
  inline void set_viewport(const Viewport &viewport) noexcept {
    set_viewports({&viewport, 1});
  }
  inline void set_scissor(const Rect &scissor) noexcept {
    set_scissors({&scissor, 1});
  }

  // ====== ShaderObjects =======
  void bind_shader(VertexShader shader) noexcept;
  void bind_shader(FragmentShader shader) noexcept;
  // TODO:
  void unbind_shaders(ShaderStage stage) noexcept;

  // ====== draw-calls ==========
  void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0) noexcept;
  void draw_indexed(uint32_t indexCount, uint32_t instanceCount = 1,
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                    uint32_t firstInstance = 0) noexcept;

private:
  CommandBuffer(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
