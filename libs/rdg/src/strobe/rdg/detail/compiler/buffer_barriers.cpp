#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
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

flow::detail::ResolvedOperation pipeline_barrier_op(BufferState source,
                                                    BufferState target) {
  flow::detail::ResolvedOperation op;
  op.metadata = flow::OpMetadata("pipeline_barrier");
  op.argument_types.push_back(flow::type_id<rhi::CommandBuffer>());
  op.argument_types.push_back(flow::type_id<rhi::Buffer>());
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
    const auto &buffer = *static_cast<rhi::Buffer *>(arguments[1]);
    const rhi::BufferBarrier barrier{
        .buffer = buffer,
        .srcStage = source.stage,
        .srcAccess = source.access,
        .dstStage = target.stage,
        .dstAccess = target.access,
    };
    command.pipeline_barrier(rhi::PipelineBarrier{
        .memoryBarriers = {},
        .bufferBarriers = &barrier,
        .imageBarriers = {},
    });
  };
  return op;
}

} // namespace

void place_buffer_barriers(flow::detail::compiler::NormalizedIR &ir,
                           flow::CompileContext &context) {
  auto &state = compiler_state(context);
  for (const auto &access : state.buffer_accesses) {
    auto &tracked = state.buffer_states[access.buffer];
    if (!tracked) {
      throw std::logic_error("buffer access is missing an initial state");
    }
    const bool has_previous_access =
        tracked->stage != rhi::PipelineStage::none ||
        tracked->access != rhi::Access::none;
    const bool barrier = writes(tracked->access) ||
                         (has_previous_access && writes(access.state.access));
    auto &node = ir.nodes[access.node];
    if (barrier) {
      node.resolved_operation = context.resolve_operation(
          pipeline_barrier_op(*tracked, access.state));
      node.intrinsic.reset();
      *tracked = access.state;
    } else {
      tracked->stage |= access.state.stage;
      tracked->access |= access.state.access;
      state.redundant_accesses[access.node] = 1;
    }
  }
}

void remove_redundant_accesses(flow::detail::compiler::NormalizedIR &ir,
                               flow::CompileContext &context) {
  erase_marked_nodes(ir, compiler_state(context).redundant_accesses);
}

} // namespace strobe::rdg::detail
