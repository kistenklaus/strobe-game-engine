#pragma once

#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/resource_descriptor_array.hpp"
#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/aabb_geometry_data.hpp"
#include "strobe/rhi/types/access_scope.hpp"
#include "strobe/rhi/types/blend_equation.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/compare_op.hpp"
#include "strobe/rhi/types/cull_mode.hpp"
#include "strobe/rhi/types/front_face.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/logic_op.hpp"
#include "strobe/rhi/types/memory_barrier.hpp"
#include "strobe/rhi/types/polygon_mode.hpp"
#include "strobe/rhi/types/primitive_topology.hpp"
#include "strobe/rhi/types/rect.hpp"
#include "strobe/rhi/types/rendering_info.hpp"
#include "strobe/rhi/types/sample_count.hpp"
#include "strobe/rhi/types/shader_stage.hpp"
#include "strobe/rhi/types/stencil_face.hpp"
#include "strobe/rhi/types/stencil_op.hpp"
#include "strobe/rhi/types/triangle_geometry_data.hpp"
#include "strobe/rhi/types/vertex_attribute.hpp"
#include "strobe/rhi/types/vertex_binding.hpp"
#include "strobe/rhi/types/viewport.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Records device commands.
 * Defined in header <strobe/rhi/rhi.hpp>
 *
 * \code{.cpp}
 * class CommandBuffer : public Object<CommandBuffer>;
 * \endcode
 *
 * Thin abstraction over a Vulkan command buffer with default rendering state
 * and descriptor-heap integration.
 *
 * \attention 1. The owning CommandPool must be externally synchronized while
 * recording commands.
 */
class CommandBuffer : public Object<CommandBuffer> {
  friend class Object<CommandBuffer>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  /**
   * \brief Begins command recording.
   * \code{.cpp}
   * void begin();
   * \endcode
   *
   * Begins recording commands into the command buffer.
   *
   * \attention 1. The command buffer must not already be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void begin();

  /**
   * \brief Ends command recording.
   * \code{.cpp}
   * void end();
   * \endcode
   *
   * Finishes recording commands into the command buffer.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. No dynamic rendering instance may be active.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void end();

  /**
   * \brief Records memory barrier.
   * \code{.cpp}
   * void memory_barrier(const MemoryBarrier& barrier);     (1)
   * void memory_barrier(AccessScope src, AccessScope dst); (2)
   * \endcode
   *
   * Establishes the memory dependency described by \p barrier.
   *
   * \param barrier Memory dependency.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   *
   * \note On modern hardware, dedicated buffer and image barriers often offer
   * little benefit over equivalent memory barriers.
   */
  void memory_barrier(const MemoryBarrier &barrier);
  void memory_barrier(AccessScope src, AccessScope dst);

  /**
   * \brief Transitions image layout.
   * \code{.cpp}
   * void transition_image(const Image& image, ImageLayout src, ImageLayout dst) noexcept;
   * \endcode
   *
   * Transitions \p image from \p src to \p dst.
   *
   * \param image Image to transition.
   * \param src Current image layout.
   * \param dst Destination image layout.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   *
   * \note ImageLayout::general provides competitive performance on most modern
   * hardware and can avoid unnecessary layout transitions.
   */
  void transition_image(const Image &image, ImageLayout src,
                        ImageLayout dst) noexcept;

  /** \name Rendering
   * @{
   */

  /**
   * \brief Begins dynamic rendering.
   * \code{.cpp}
   * void begin_rendering(const RenderingInfo& info) noexcept;
   * \endcode
   *
   * Begins rendering using the attachments described by \p info.
   *
   * \param info Rendering configuration.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. No dynamic rendering instance may already be active.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void begin_rendering(const RenderingInfo &info) noexcept;

  /**
   * \brief Ends dynamic rendering.
   * \code{.cpp}
   * void end_rendering() noexcept;
   * \endcode
   *
   * Ends the currently active rendering instance.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. A dynamic rendering instance must be active.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void end_rendering() noexcept;

  /**
   * \brief Sets rendering viewports.
   * \code{.cpp}
   * void set_viewports(span<const Viewport> viewports) noexcept;
   * void set_viewport(const Viewport& viewports) noexcept;
   * \endcode
   *
   * Replaces the active viewport state.
   *
   * \param viewports Viewports to set.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_viewports(span<const Viewport> viewports) noexcept;
  inline void set_viewport(const Viewport &viewport) noexcept {
    set_viewports({&viewport, 1});
  }

  /**
   * \brief Sets scissor rectangles.
   * \code{.cpp}
   * void set_scissors(span<const Rect> scissors) noexcept;
   * void set_scissor(const Rect scissors) noexcept;
   * \endcode
   *
   * Replaces the active scissor state.
   *
   * \param scissors Scissor rectangles to set.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_scissors(span<const Rect> scissors) noexcept;
  inline void set_scissor(const Rect &scissor) noexcept {
    set_scissors({&scissor, 1});
  }

  /**
   * \brief Sets rasterizer discard.
   * \code{.cpp}
   * void set_rasterizer_discard_enable(bool discardEnable) noexcept;
   * \endcode
   *
   * Controls whether primitives are discarded before rasterization.
   * Rasterizer discard is disabled when recording begins.
   *
   * \param discardEnable Whether rasterizer discard is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_rasterizer_discard_enable(bool discardEnable) noexcept;

  /**
   * \brief Sets primitive topology.
   * \code{.cpp}
   * void set_primitive_topology(PrimitiveTopology topology) noexcept;
   * \endcode
   *
   * Sets the topology used to assemble primitives.
   *
   * \param topology Primitive topology.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_primitive_topology(PrimitiveTopology topology) noexcept;

  /**
   * \brief Sets primitive restart.
   * \code{.cpp}
   * void set_primitive_restart(bool restartEnable) noexcept;
   * \endcode
   *
   * Controls primitive restart during indexed rendering.
   *
   * \param restartEnable Whether primitive restart is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_primitive_restart(bool restartEnable) noexcept;

  /**
   * \brief Sets cull mode.
   * \code{.cpp}
   * void set_cull_mode(CullMode cullMode) noexcept;
   * \endcode
   *
   * Selects which polygon faces are discarded during rasterization.
   *
   * \param cullMode Faces to cull.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_cull_mode(CullMode cullMode) noexcept;

  /**
   * \brief Sets front face.
   * \code{.cpp}
   * void set_front_face(FrontFace frontFace) noexcept;
   * \endcode
   *
   * Selects the winding order considered front-facing.
   *
   * \param frontFace Front-facing winding order.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_front_face(FrontFace frontFace) noexcept;

  /**
   * \brief Sets depth bias.
   * \code{.cpp}
   * void set_depth_bias_enable(bool depthBiasEnable) noexcept;
   * \endcode
   *
   * Controls whether depth bias is applied during rasterization.
   *
   * \param depthBiasEnable Whether depth bias is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_bias_enable(bool depthBiasEnable) noexcept;

  /**
   * \brief Sets depth testing.
   * \code{.cpp}
   * void set_depth_test_enable(bool depthTestEnable) noexcept;
   * \endcode
   *
   * Controls whether depth testing is performed.
   *
   * \param depthTestEnable Whether depth testing is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_test_enable(bool depthTestEnable) noexcept;

  /**
   * \brief Sets depth writes.
   * \code{.cpp}
   * void set_depth_write_enable(bool depthWriteEnable) noexcept;
   * \endcode
   *
   * Controls whether passing fragments update the depth attachment.
   *
   * \param depthWriteEnable Whether depth writes are enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_write_enable(bool depthWriteEnable) noexcept;

  /**
   * \brief Sets depth comparison.
   * \code{.cpp}
   * void set_depth_compare_op(CompareOp op) noexcept;
   * \endcode
   *
   * Selects the comparison operation used by the depth test.
   *
   * \param op Depth comparison operation.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_compare_op(CompareOp op) noexcept;

  /**
   * \brief Sets depth bounds.
   * \code{.cpp}
   * void set_depth_bounds_test_enable(bool depthBoundTestEnable) noexcept;
   * \endcode
   *
   * Controls whether depth-bounds testing is performed.
   *
   * \param depthBoundTestEnable Whether depth-bounds testing is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_bounds_test_enable(bool depthBoundTestEnable) noexcept;

  /**
   * \brief Sets depth range.
   * \code{.cpp}
   * void set_depth_bounds(float minDepthBound, float maxDepthBound) noexcept;
   * \endcode
   *
   * Sets the bounds used by depth-bounds testing.
   *
   * \param minDepthBound Minimum depth bound.
   * \param maxDepthBound Maximum depth bound.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p minDepthBound must not exceed \p maxDepthBound.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_depth_bounds(float minDepthBound, float maxDepthBound) noexcept;

  /**
   * \brief Sets stencil testing.
   * \code{.cpp}
   * void set_stencil_test_enable(bool stencilTestEnable) noexcept;
   * \endcode
   *
   * Controls whether stencil testing is performed.
   *
   * \param stencilTestEnable Whether stencil testing is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_stencil_test_enable(bool stencilTestEnable) noexcept;

  /**
   * \brief Sets stencil operations.
   * \code{.cpp}
   * void set_stencil_op(StencilFace faceMask, StencilOp failOp, StencilOp passOp, StencilOp depthFail, CompareOp compareOp);
   * \endcode
   *
   * Sets stencil behavior for the selected faces.
   *
   * \param faceMask Stencil faces to modify.
   * \param failOp Operation when the stencil test fails.
   * \param passOp Operation when stencil and depth tests pass.
   * \param depthFail Operation when the depth test fails.
   * \param compareOp Stencil comparison operation.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_stencil_op(StencilFace faceMask, StencilOp failOp, StencilOp passOp,
                      StencilOp depthFail, CompareOp compareOp) noexcept;

  /**
   * \brief Sets stencil compare mask.
   * \code{.cpp}
   * void set_stencil_compare_mask(StencilFace faceMask, uint32_t compareMask) noexcept;
   * \endcode
   * 
   * Sets the comparison mask for the selected stencil faces.
   *
   * \param faceMask Stencil faces to modify.
   * \param compareMask Stencil comparison mask.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_stencil_compare_mask(StencilFace faceMask,
                                uint32_t compareMask) noexcept;

  /**
   * \brief Sets stencil write mask.
   * \code{.cpp}
   * void set_stencil_write_mask(StencilFace faceMask, uint32_t writeMask) noexcept;
   * \endcode
   *
   * Sets the write mask for the selected stencil faces.
   *
   * \param faceMask Stencil faces to modify.
   * \param writeMask Stencil write mask.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_stencil_write_mask(StencilFace faceMask,
                              uint32_t writeMask) noexcept;

  /**
   * \brief Sets stencil reference.
   * \code{.cpp}
   * void set_stencil_reference(StencilFace faceMask, uint32_t reference) noexcept;
   * \endcode
   *
   * Sets the reference value for the selected stencil faces.
   *
   * \param faceMask Stencil faces to modify.
   * \param reference Stencil reference value.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_stencil_reference(StencilFace faceMask, uint32_t reference) noexcept;

  /**
   * \brief Sets blend constants.
   * \code{.cpp}
   * void set_blend_constants(vec4 constants) noexcept;
   * \endcode
   *
   * Sets the constant RGBA values used by blending.
   *
   * \param constants Blend constants.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_blend_constants(vec4 constants) noexcept;

  /**
   * \brief Sets line width.
   * \code{.cpp}
   * void set_line_width(float lineWidth) noexcept;
   * \endcode
   *
   * Sets the width used when rasterizing lines.
   *
   * \param lineWidth Line width in pixels.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p lineWidth must be greater than zero.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_line_width(float lineWidth) noexcept;

  /**
   * \brief Sets depth bias.
   * \code{.cpp}
   * void set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlope) noexcept;
   * \endcode
   *
   * Sets the parameters used when depth bias is enabled.
   *
   * \param depthBiasConstantFactor Constant depth-bias factor.
   * \param depthBiasClamp Maximum depth-bias magnitude.
   * \param depthBiasSlope Slope-dependent depth-bias factor.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp,
                      float depthBiasSlope) noexcept;

  /**
   * \brief Sets vertex input.
   * \code{.cpp}
   * void set_vertex_input(span<const VertexBinding> bindings, span<const VertexAttribute> attributes) noexcept;
   * \endcode
   *
   * Sets vertex bindings and their associated attributes.
   *
   * \param bindings Vertex buffer bindings.
   * \param attributes Vertex attributes.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. Each attribute must reference a valid entry in \p bindings.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_vertex_input(span<const VertexBinding> bindings,
                        span<const VertexAttribute> attributes) noexcept;

  /**
   * \brief Sets rasterization samples.
   *
   * Sets the sample count used during rasterization.
   *
   * \param rasterizationSamples Rasterization sample count.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_rasterization_samples(SampleCount rasterizationSamples) noexcept;

  /**
   * \brief Sets sample mask.
   * \code{.cpp}
   * void set_sample_mask(SampleCount samples, uint64_t mask = std::numeric_limits<uint64_t>::max()) noexcept;
   * \endcode
   *
   * Selects which rasterization samples are enabled.
   *
   * \param samples Rasterization sample count.
   * \param mask Enabled sample bits.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p samples must match the active rasterization sample count.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_sample_mask(
      SampleCount samples,
      uint64_t mask = std::numeric_limits<uint64_t>::max()) noexcept;

  /**
   * \brief Sets alpha-to-coverage.
   * \code{.cpp}
   * void set_alpha_to_coverage_enable(bool alphaToCoverageEnable) noexcept;
   * \endcode
   *
   * Controls alpha-to-coverage multisampling.
   *
   * \param alphaToCoverageEnable Whether alpha-to-coverage is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_alpha_to_coverage_enable(bool alphaToCoverageEnable) noexcept;

  /**
   * \brief Sets polygon mode.
   * \code{.cpp}
   * void set_polygon_mode(PolygonMode polygonMode) noexcept;
   * \endcode
   *
   * Selects how polygons are rasterized.
   *
   * \param polygonMode Polygon rasterization mode.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_polygon_mode(PolygonMode polygonMode) noexcept;

  /**
   * \brief Sets depth clamp.
   * \code{.cpp}
   * void set_depth_clamp_enable(bool depthClampEnable) noexcept;
   * \endcode
   *
   * Controls whether fragment depth values are clamped.
   *
   * \param depthClampEnable Whether depth clamping is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_depth_clamp_enable(bool depthClampEnable) noexcept;

  /**
   * \brief Sets logic operations.
   * \code{.cpp}
   * void set_logic_op_enable(bool logicOpEnable) noexcept;
   * \endcode
   *
   * Controls whether logical color operations are performed.
   *
   * \param logicOpEnable Whether logic operations are enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_logic_op_enable(bool logicOpEnable) noexcept;

  /**
   * \brief Sets logic operation.
   * \code{.cpp}
   * void set_logic_op(LogicOp logicOp) noexcept;
   * \endcode
   *
   * Selects the logical operation applied to color values.
   *
   * \param logicOp Logical color operation.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_logic_op(LogicOp logicOp) noexcept;

  /**
   * \brief Sets color blending.
   * \code{.cpp}
   * void set_color_blend_enable(uint32_t firstAttachment, uint32_t attachmentCount, uint32_t bitmask) noexcept; (1)
   * void set_color_blend_enable(uint32_t firstAttachment, uint32_t attachmentCount, bool enable) noexcept; (2)
   * \endcode
   *
   * Sets blending enable state for consecutive color attachments.
   *
   * \param firstAttachment First color attachment.
   * \param attachmentCount Number of attachments.
   * \param bitmask Blending enable bits.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p attachmentCount must not exceed the number of bits
   * represented by \p bitmask.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_color_blend_enable(uint32_t firstAttachment,
                              uint32_t attachmentCount,
                              uint32_t bitmask) noexcept;
  inline void set_color_blend_enable(uint32_t firstAttachment,
                                     uint32_t attachmentCount,
                                     bool enable) noexcept {
    set_color_blend_enable(firstAttachment, attachmentCount,
                           enable ? std::numeric_limits<uint32_t>::max() : 0);
  }

  /**
   * \brief Sets blend equations.
   * \code{.cpp}
   * void set_color_blend_equation(uint32_t firstAttachment, span<const BlendEquation> colorBlendEquations) noexcept;
   * \endcode
   *
   * Sets one blend equation for each consecutive color attachment.
   *
   * \param firstAttachment First color attachment.
   * \param colorBlendEquations Blend equations.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The attachment range must be valid for the active rendering
   * configuration.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_color_blend_equation(
      uint32_t firstAttachment,
      span<const BlendEquation> colorBlendEquations) noexcept;

  /**
   * \brief Sets color write masks.
   * \code{.cpp}
   * void set_color_write_mask(uint32_t firstAttachment, span<const ColorComponent> colorWriteMasks) noexcept;
   * \endcode
   *
   * Sets one component write mask for each consecutive color attachment.
   *
   * \param firstAttachment First color attachment.
   * \param colorWriteMasks Color component masks.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The attachment range must be valid for the active rendering
   * configuration.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void
  set_color_write_mask(uint32_t firstAttachment,
                       span<const ColorComponent> colorWriteMasks) noexcept;

  /**
   * \brief Sets alpha-to-one.
   * \code{.cpp}
   * void set_alpha_to_one_enable(bool alphaToOneEnable) noexcept;
   * \endcode
   *
   * Controls alpha-to-one multisampling.
   *
   * \param alphaToOneEnable Whether alpha-to-one is enabled.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void set_alpha_to_one_enable(bool alphaToOneEnable) noexcept;

  /**
   * \brief Sets patch control points.
   * \code{.cpp}
   * void set_patch_control_points(uint32_t patchControlPoints) noexcept;
   * \endcode
   *
   * Sets the number of control points used by patch primitives.
   *
   * \param patchControlPoints Number of control points.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p patchControlPoints must be greater than zero.
   * \attention 3. The owning CommandPool must be externally synchronized.
   */
  void set_patch_control_points(uint32_t patchControlPoints) noexcept;




  /**
   * \brief Binds vertex buffer.
   * \code{.cpp}
   * void bind_vertex_buffer(const Buffer& buffer, uint64_t offset = 0) noexcept;
   * \endcode
   *
   * Binds \p buffer as vertex input starting at \p offset.
   *
   * \param buffer Vertex buffer.
   * \param offset Byte offset into the buffer.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. \p buffer must support vertex-buffer usage.
   * \attention 3. \p offset must refer to a valid location within \p buffer.
   * \attention 4. The owning CommandPool must be externally synchronized.
   */
  void bind_vertex_buffer(const Buffer &buffer, uint64_t offset = 0) noexcept;

  /** @} */

  /** \name Shader objects
   * @{
   */

  /**
   * \brief Binds vertex shader.
   * \code{.cpp}
   * void bind_shader(const VertexShader& shader) noexcept;   (1)
   * void bind_shader(const FragmentShader& shader) noexcept; (2)
   * void bind_shader(const ComputeShader& shader) noexcept;  (3)
   * \endcode
   *
   * Binds \p shader to the vertex stage.
   *
   * \param shader Vertex shader.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void bind_shader(const VertexShader &shader) noexcept;
  void bind_shader(const FragmentShader &shader) noexcept;
  void bind_shader(const ComputeShader &shader) noexcept;

  /**
   * \brief Unbinds shader stages.
   * \code{.cpp}
   * void unbind_shader(ShaderStage stage) noexcept;
   * \endcode
   *
   * Unbinds shaders from the stages selected by \p stage.
   *
   * \param stage Shader stages to unbind.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The owning CommandPool must be externally synchronized.
   */
  void unbind_shaders(ShaderStage stage) noexcept;

  /** @} */

  /** \name Transfers
   * @{
   */

  /**
   * \brief Copies buffer data.
   * \code{.cpp}
   * void copy_buffer(BufferOffset dst, BufferOffset src, uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
   * \endcode
   *
   * Copies \p size bytes from \p src to \p dst.
   *
   * \param dst Destination buffer and offset.
   * \param src Source buffer and offset.
   * \param size Number of bytes to copy.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. No dynamic rendering instance may be active.
   * \attention 3. Source and destination ranges must be valid.
   * \attention 4. Source and destination ranges must not overlap when they
   * refer to the same underlying buffer.
   * \attention 5. The source and destination buffers must support the required
   * transfer usage.
   * \attention 6. The owning CommandPool must be externally synchronized.
   */
  void
  copy_buffer(BufferOffset dst, BufferOffset src,
              uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  /**
   * \brief Updates buffer data.
   * \code{.cpp}
   * void update(BufferOffset dst, const void* src, uint64_t size) noexcept; (1)
   * template<typename T>
   * void update(BufferOffset dst, span<const T> src) noexcept;              (2)
   * \endcode
   *
   * Copies host data into a buffer as part of command execution.
   *
   * \param dst Destination buffer and offset.
   * \param src Source data.
   * \param size Number of bytes to copy.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. No dynamic rendering instance may be active.
   * \attention 3. The destination range must be valid.
   * \attention 4. The destination buffer must support transfer-destination
   * usage.
   * \attention 5. Vulkan update-buffer size and alignment restrictions apply.
   * \attention 6. The owning CommandPool must be externally synchronized.
   */
  void update(BufferOffset dst, const void *src, uint64_t size) noexcept;
  template <typename T>
  inline void update(BufferOffset dst, span<const T> src) noexcept {
    update(std::move(dst), src.data(), src.size_bytes());
  }

  /** @} */

  /** \name Drawing
   * @{
   */

  /**
   * \brief Records draw call.
   * \code{.cpp}
   * void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) noexcept;
   * \endcode
   *
   * Draws non-indexed primitives using the current graphics state.
   *
   * \param vertexCount Number of vertices.
   * \param instanceCount Number of instances.
   * \param firstVertex First vertex.
   * \param firstInstance First instance.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. A dynamic rendering instance must be active.
   * \attention 3. Required graphics shader stages and dynamic state must be
   * valid.
   * \attention 4. The owning CommandPool must be externally synchronized.
   */
  void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0) noexcept;

  /**
   * \brief Records indexed draw.
   * \code{.cpp}
   * void draw_indexed(uint32_t indexCount, 
   *                   uint32_t instanceCount = 1, 
   *                   uint32_t firstIndex = 0, 
   *                   uint32_t firstIndex = 0, 
   *                   uint32_t vertexOffset = 0, 
   *                   uint32_t firstInstance = 0) noexcept;
   * \endcode
   *
   * Draws indexed primitives using the current graphics state.
   *
   * \param indexCount Number of indices.
   * \param instanceCount Number of instances.
   * \param firstIndex First index.
   * \param vertexOffset Offset added to each vertex index.
   * \param firstInstance First instance.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. A dynamic rendering instance must be active.
   * \attention 3. A valid index buffer must be bound.
   * \attention 4. Required graphics shader stages and dynamic state must be
   * valid.
   * \attention 5. The owning CommandPool must be externally synchronized.
   */
  void draw_indexed(uint32_t indexCount, uint32_t instanceCount = 1,
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                    uint32_t firstInstance = 0) noexcept;

  /** @} */

  /** \name Acceleration structures
   * @{
   */

  /**
   * \brief Builds acceleration structure.
   * \code{.cpp}
   * void build(const Blas& blas, span<const TriangleGeometryData> triangleGeometries) noexcept; (1)
   * void build(const Blas& blas, span<const AabbGeometryData> aabbGeometries) noexcept;         (2)
   * void build(const Tlas& tlas, BufferOffset instanceBuffer, uint32_t count) noexcept;         (3)
   * \endcode
   *
   * Builds \p blas from triangle geometry.
   *
   * \param blas Destination acceleration structure.
   * \param triangleGeometries Triangle geometry.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. No dynamic rendering instance may be active.
   * \attention 3. Geometry buffers must remain valid until execution
   * completes.
   * \attention 4. The owning CommandPool must be externally synchronized.
   */
  void build(const Blas &blas,
             span<const TriangleGeometryData> triangleGeometries) noexcept;
  void build(const Blas &blas,
             span<const AabbGeometryData> aabbGeometries) noexcept;
  void build(const Tlas &tlas, BufferOffset instanceBuffer,
             uint32_t count) noexcept;

  /** @} */

  /** \name Push constants
   * @{
   */

  /**
   * \brief Pushes constant data.
   * \code{.cpp}
   * void push(uint32_t offset, void* data, uint32_t size) noexcept;                 (1)
   * void push(uint32_t offset, const ResourceDescriptor& descriptor) noexcept;      (2)
   * void push(uint32_t offset, const ResourceDescriptorArray& descriptor) noexcept; (3)
   * \endcode
   *
   * Writes \p size bytes into the shader push-constant block.
   *
   * \param offset Byte offset.
   * \param data Source data.
   * \param size Number of bytes.
   *
   * \attention 1. The command buffer must be recording.
   * \attention 2. The written range must fit within the supported push-constant
   * block.
   * \attention 3. Vulkan push-constant alignment requirements apply.
   * \attention 4. The owning CommandPool must be externally synchronized.
   */
  void push(uint32_t offset, void *data, uint32_t size) noexcept;
  void push(uint32_t offset, const ResourceDescriptor &descriptor) noexcept;
  void push(uint32_t offset,
            const ResourceDescriptorArray &descriptor) noexcept;

  /** @} */
};

} // namespace strobe::rhi
