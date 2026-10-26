#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/queue.hpp"

#include <stdexcept>

namespace strobe::rdg::detail {

void define_globals(flow::detail::compiler::NormalizedIR &ir,
                    flow::CompileContext &context) {
  auto &state = context.emplace_plugin_data<CompilerState>(compiler_state_id);
  Vector<std::uint8_t, flow::allocator_ref> removed(
      ir.nodes.size(), std::uint8_t{0}, flow::allocator_ref{});

  for (flow::detail::NodeId node_id = 0; node_id < ir.nodes.size(); ++node_id) {
    const auto &node = ir.nodes[node_id];
    if (intrinsic(node, "def_device")) {
      if (state.device != flow::null_resource_id) {
        throw std::logic_error("RDG has multiple device definitions");
      }
      state.device = single_resource(ir, node, flow::type_id<rhi::Device>(),
                                     flow::AccessMode::read_write);
      removed[node_id] = 1;
      continue;
    }
    if (intrinsic(node, "def_queue")) {
      if (state.queue != flow::null_resource_id) {
        throw std::logic_error("RDG has multiple queue definitions");
      }
      state.queue = single_resource(ir, node, flow::type_id<rhi::Queue>(),
                                    flow::AccessMode::read_write);
      removed[node_id] = 1;
      continue;
    }
  }

  erase_marked_nodes(ir, removed);
}

} // namespace strobe::rdg::detail
