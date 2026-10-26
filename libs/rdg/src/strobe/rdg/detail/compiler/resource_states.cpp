#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/image.hpp"

#include <optional>
#include <stdexcept>

namespace strobe::rdg::detail {

void collect_resource_states(flow::detail::compiler::NormalizedIR &ir,
                             flow::CompileContext &context) {
  auto &state = compiler_state(context);
  state.image_states = decltype(state.image_states)(
      ir.resources.size(), std::nullopt, flow::allocator_ref{});
  state.buffer_states = decltype(state.buffer_states)(
      ir.resources.size(), std::nullopt, flow::allocator_ref{});
  Vector<std::uint8_t, flow::allocator_ref> removed(
      ir.nodes.size(), std::uint8_t{0}, flow::allocator_ref{});

  for (flow::detail::NodeId node_id = 0; node_id < ir.nodes.size(); ++node_id) {
    const auto &node = ir.nodes[node_id];
    if (intrinsic(node, "decl-image-state")) {
      const auto image = single_resource(ir, node, flow::type_id<rhi::Image>(),
                                         flow::AccessMode::read);
      if (state.image_states[image]) {
        throw std::logic_error("image has multiple initial-state declarations");
      }
      const auto &operation = *node.intrinsic;
      state.image_states[image] = ImageState{
          .stage = operation.immediate<rhi::PipelineStage>(0),
          .access = operation.immediate<rhi::Access>(1),
          .layout = operation.immediate<rhi::ImageLayout>(2),
      };
      removed[node_id] = 1;
      continue;
    }
    if (intrinsic(node, "decl-buffer-state")) {
      const auto buffer = single_resource(
          ir, node, flow::type_id<rhi::Buffer>(), flow::AccessMode::read);
      if (state.buffer_states[buffer]) {
        throw std::logic_error(
            "buffer has multiple initial-state declarations");
      }
      const auto &operation = *node.intrinsic;
      state.buffer_states[buffer] = BufferState{
          .stage = operation.immediate<rhi::PipelineStage>(0),
          .access = operation.immediate<rhi::Access>(1),
      };
      removed[node_id] = 1;
    }
  }

  erase_marked_nodes(ir, removed);
}

} // namespace strobe::rdg::detail
