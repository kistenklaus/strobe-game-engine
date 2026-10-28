#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/types/pipeline_barrier.hpp"

#include <cassert>
#include <stdexcept>

namespace strobe::rdg::detail {
namespace {

bool writes(rhi::Access access) noexcept {
  constexpr auto write_access =
      rhi::Access::shader_storage_write | rhi::Access::color_attachment_write |
      rhi::Access::depth_stencil_attachment_write |
      rhi::Access::transfer_write | rhi::Access::host_write |
      rhi::Access::memory_write;
  return (access & write_access) != 0;
}

flow::detail::ResolvedOperation pipeline_barrier_op(ImageState source,
                                                    ImageState target) {
  flow::detail::ResolvedOperation op;
  op.metadata = flow::OpMetadata("pipeline_barrier");
  op.argument_types.push_back(flow::type_id<rhi::CommandBuffer>());
  op.argument_types.push_back(flow::type_id<rhi::Image>());
  op.argument_groups.push_back({0, 1, 0, 1});
  op.argument_groups.push_back({1, 1, 1, 1});
  op.argument_elements.push_back(0);
  op.argument_elements.push_back(1);
  op.argument_accesses.push_back(flow::AccessMode::read_write);
  op.argument_accesses.push_back(flow::AccessMode::read);
  op.runtime = [source, target](flow::Lifetime, span<void *const>,
                               span<const std::uint32_t>,
                               span<const flow::ArgumentGroup>,
                               span<const std::uint32_t>,
                               span<void *const> arguments) noexcept {
    assert(arguments.size() == 2);
    auto &command = *static_cast<rhi::CommandBuffer *>(arguments[0]);
    const auto &image = *static_cast<rhi::Image *>(arguments[1]);
    const rhi::ImageBarrier barrier{
        .image = image,
        .srcStage = source.stage,
        .srcAccess = source.access,
        .srcLayout = source.layout,
        .dstStage = target.stage,
        .dstAccess = target.access,
        .dstLayout = target.layout,
    };
    command.pipeline_barrier(rhi::PipelineBarrier{
        .memoryBarriers = {},
        .bufferBarriers = {},
        .imageBarriers = &barrier,
    });
  };
  return op;
}

} // namespace

void place_image_barriers(flow::detail::compiler::NormalizedIR &ir,
                          flow::CompileContext &context) {
  auto &state = compiler_state(context);
  for (const auto &marker : state.image_markers) {
    auto &tracked = state.image_states[marker.image];
    if (marker.kind == ImageMarkerKind::discard) {
      tracked = ImageState{
          .stage = rhi::PipelineStage::none,
          .access = rhi::Access::none,
          .layout = rhi::ImageLayout::undefined,
      };
      state.removed_markers[marker.node] = 1;
      continue;
    }
    if (!tracked) {
      throw std::logic_error("image access is missing an initial state");
    }
    const bool has_previous_access =
        tracked->stage != rhi::PipelineStage::none ||
        tracked->access != rhi::Access::none;
    const bool barrier = tracked->layout != marker.state.layout ||
                         writes(tracked->access) ||
                         (has_previous_access && writes(marker.state.access));
    auto &node = ir.nodes[marker.node];
    if (barrier) {
      node.resolved_operation = context.resolve_operation(
          pipeline_barrier_op(*tracked, marker.state));
      node.intrinsic.reset();
      *tracked = marker.state;
    } else {
      tracked->stage |= marker.state.stage;
      tracked->access |= marker.state.access;
      state.removed_markers[marker.node] = 1;
    }
  }
}

} // namespace strobe::rdg::detail
