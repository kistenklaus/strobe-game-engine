#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/rhi/cmd/command_buffer_rendering_state.hpp"
#include "strobe/rhi/cmd/command_buffer_state.hpp"
#include "strobe/rhi/cmd/native_command_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/stage/stage_arena.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"
#include "strobe/rhi/vulkan/cmd/rendering.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include <limits>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>
#ifdef STROBE_TRACY
#include "strobe/rhi/vulkan/tracy/profiler.hpp"
#endif

namespace strobe::rhi {

struct CommandBufferImpl {

  explicit CommandBufferImpl(CommandPool pool, StagingPool stagePool,
                             NativeCommandPool *nativePool,
                             vulkan::CommandBuffer cmd,
                             CommandBufferFlags flags,
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
  const vulkan::Context *ctx;

  StageArena localStage;

  // profiling
#ifdef STROBE_TRACY
  [[maybe_unused]] profiler::CommandBufferScope m_profilerScope;
#endif

  // dynamic rendering state
  uint32_t renderingColorAttachmentCount = std::numeric_limits<uint32_t>::max();
  CommandBufferRenderingState uninitialized = CommandBufferRenderingState::all;
  CommandBufferRenderingState required =
      CommandBufferRenderingState::graphics_pipeline_requirements;

  void
  set_default_rendering_state(CommandBufferRenderingState states) noexcept {
    auto mask =
        static_cast<std::underlying_type_t<CommandBufferRenderingState>>(
            states);
    while (mask != 0) {
      const uint32_t index = static_cast<uint32_t>(std::countr_zero(mask));
      const auto renderingCmd = static_cast<CommandBufferRenderingCmd>(index);
      switch (renderingCmd) {
      case CommandBufferRenderingCmd::rasterizer_discard_enable:
        vulkan::cmd_set_rasterizer_discard_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::primitive_topology:
        vulkan::cmd_set_primitive_topology(cmd,
                                           VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        break;
      case CommandBufferRenderingCmd::primitive_restart_enable:
        vulkan::cmd_set_primitive_restart(cmd, false);
        break;
      case CommandBufferRenderingCmd::cull_mode:
        vulkan::cmd_set_cull_mode(cmd, VK_CULL_MODE_NONE);
        break;
      case CommandBufferRenderingCmd::front_face:
        vulkan::cmd_set_front_face(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        break;
      case CommandBufferRenderingCmd::depth_bias_enable:
        vulkan::cmd_set_depth_bias_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::depth_test_enable:
        vulkan::cmd_set_depth_test_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::depth_write_enable:
        vulkan::cmd_set_depth_write_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::depth_compare_op:
        vulkan::cmd_set_depth_compare_op(cmd, VK_COMPARE_OP_LESS);
        break;
      case CommandBufferRenderingCmd::depth_bounds_test_enable:
        vulkan::cmd_set_depth_bounds_test_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::depth_bounds:
        vulkan::cmd_set_depth_bounds(cmd, 0.0f, 1.0f);
        break;
      case CommandBufferRenderingCmd::stencil_test_enable:
        vulkan::cmd_set_stencil_test_enable(cmd, false);
        break;
      case CommandBufferRenderingCmd::stencil_compare_mask:
        vulkan::cmd_set_stencil_compare_mask(
            cmd, VK_STENCIL_FACE_FRONT_AND_BACK, 0xffffffffu);
        break;
      case CommandBufferRenderingCmd::vertex_input:
        vulkan::cmd_set_vertex_input(
            ctx, cmd, span<const VkVertexInputBindingDescription2EXT>{}, {});
        break;
      case CommandBufferRenderingCmd::rasterization_samples: {
        const bool sampleMaskUninitialized =
            (uninitialized & CommandBufferRenderingState::sample_mask) != 0;
        vulkan::cmd_set_rasterizer_samples(ctx, cmd, VK_SAMPLE_COUNT_1_BIT);
        if (sampleMaskUninitialized) {
          vulkan::cmd_set_sample_mask(ctx, cmd, VK_SAMPLE_COUNT_1_BIT,
                                      std::numeric_limits<uint64_t>::max());
        }
        break;
      }
      case CommandBufferRenderingCmd::sample_mask:
        break; // just let validation catch me
      case CommandBufferRenderingCmd::alpha_to_coverage_enable:
        vulkan::cmd_set_alpha_to_coverage_enable(ctx, cmd, false);
        break;
      case CommandBufferRenderingCmd::polygon_mode:
        vulkan::cmd_set_polygon_mode(ctx, cmd, VK_POLYGON_MODE_FILL);
        break;
      case CommandBufferRenderingCmd::depth_clamp_enable:
        vulkan::cmd_set_depth_clamp_enable(ctx, cmd, false);
        break;
      case CommandBufferRenderingCmd::logic_op_enable:
        vulkan::cmd_set_logic_op_enable(ctx, cmd, false);
        break;
      case CommandBufferRenderingCmd::color_blend_enable:
        assert(renderingColorAttachmentCount !=
                   std::numeric_limits<uint32_t>::max() &&
               "likely a draw-call outside of rendering");
        vulkan::cmd_set_color_blend_enable(ctx, cmd, 0,
                                           renderingColorAttachmentCount, 0);
        break;
      case CommandBufferRenderingCmd::color_write_mask: {
        assert(renderingColorAttachmentCount !=
                   std::numeric_limits<uint32_t>::max() &&
               "likely a draw-call outside of rendering");
        SmallVector<ColorComponent, 4> components{renderingColorAttachmentCount,
                                                  ColorComponent::rgba};
        vulkan::cmd_set_color_write_mask(ctx, cmd, 0, components);
        break;
      }
      case CommandBufferRenderingCmd::alpha_to_one_enable:
        vulkan::cmd_set_alpha_to_one_enable(ctx, cmd, false);
        break;
      case CommandBufferRenderingCmd::count:
        std::unreachable();
      }
      // Remove lowest set bit.
      mask &= mask - 1;
    }
    uninitialized &= ~states;
  }
};

} // namespace strobe::rhi
