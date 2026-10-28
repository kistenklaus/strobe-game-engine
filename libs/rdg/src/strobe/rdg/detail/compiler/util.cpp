#include "util.hpp"

#include <stdexcept>

namespace strobe::rdg::detail {

bool intrinsic(const flow::detail::compiler::NormalizedNode &node,
               std::string_view opcode) {
  return node.intrinsic && node.intrinsic->domain.view() == "strobe.rdg" &&
         node.intrinsic->opcode.view() == opcode &&
         node.intrinsic->version == 1;
}

flow::ResourceId
single_resource(const flow::detail::compiler::NormalizedIR &ir,
                const flow::detail::compiler::NormalizedNode &node,
                flow::TypeId type, flow::AccessMode mode) {
  const auto accesses = ir.accesses_of(node);
  if (accesses.size() != 1) {
    throw std::logic_error("RDG intrinsic requires one resource");
  }
  const auto &access = accesses.front();
  if (access.resource >= ir.resources.size() ||
      ir.resources[access.resource].type != type || access.mode != mode ||
      access.purpose != flow::AccessPurpose::argument) {
    throw std::logic_error("RDG intrinsic resource contract is invalid");
  }
  return access.resource;
}

CompilerState &compiler_state(flow::CompileContext &context) {
  return context.plugin_data<CompilerState>(compiler_state_id);
}

flow::detail::compiler::SourceOperationId
submission_for(const Vector<SubmitMarker, flow::allocator_ref> &submissions,
               flow::ResourceId command,
               flow::detail::compiler::SourceOperationId access_position) {
  const auto submit = std::ranges::find_if(submissions, [&](const auto &entry) {
    return entry.command_buffer == command && access_position < entry.position;
  });
  if (submit == submissions.end()) {
    throw std::logic_error(
        "RDG resource access has no following command-buffer submission");
  }
  return submit->position;
}

void erase_marked_nodes(
    flow::detail::compiler::NormalizedIR &ir,
    const Vector<std::uint8_t, flow::allocator_ref> &removed) {
  using namespace flow::detail;
  using namespace flow::detail::compiler;
  if (removed.size() != ir.nodes.size()) {
    throw std::logic_error("RDG node removal table has invalid size");
  }

  Vector<NodeId, flow::allocator_ref> remap(ir.nodes.size(), null_node_id,
                                            flow::allocator_ref{});
  Vector<NormalizedNode, flow::allocator_ref> nodes;
  nodes.reserve(ir.nodes.size());
  for (NodeId old = 0; old < ir.nodes.size(); ++old) {
    if (removed[old] != 0) {
      continue;
    }
    remap[old] = static_cast<NodeId>(nodes.size());
    nodes.push_back(std::move(ir.nodes[old]));
  }
  ir.nodes = std::move(nodes);

  for (auto &phi : ir.phis) {
    if (phi.node >= remap.size() || remap[phi.node] == null_node_id) {
      throw std::logic_error("RDG pass removed a phi node");
    }
    phi.node = remap[phi.node];
  }
  for (auto &cutoff : ir.cutoffs) {
    if (cutoff.node >= remap.size() || remap[cutoff.node] == null_node_id) {
      throw std::logic_error("RDG pass removed a cutoff node");
    }
    cutoff.node = remap[cutoff.node];
  }
}

} // namespace strobe::rdg::detail
