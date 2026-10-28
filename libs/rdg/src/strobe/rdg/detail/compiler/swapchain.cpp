#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rdg/swapchain.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/queue.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace strobe::rdg::detail {

flow::detail::ResolvedOperation wait_swapchain_op() {
  flow::detail::ResolvedOperation op;
  op.metadata = flow::OpMetadata("wait for swapchain image", {},
                                 flow::OpSemantics::side_effect);
  op.external = flow::external_op("swapchain", "wait", 1);
  op.argument_types.push_back(flow::type_id<rhi::Queue>());
  op.argument_types.push_back(
      flow::type_id<::strobe::rdg::swapchain_image_runtime>());
  op.argument_accesses.push_back(flow::AccessMode::read);
  op.argument_accesses.push_back(flow::AccessMode::read);
  op.argument_groups.push_back({0, 1, 0, 1});
  op.argument_groups.push_back({1, 1, 1, 1});
  op.argument_elements.push_back(0);
  op.argument_elements.push_back(1);
  return op;
}

flow::detail::ResolvedOperation present_swapchain_op() {
  flow::detail::ResolvedOperation op;
  op.metadata = flow::OpMetadata("present swapchain image", {},
                                 flow::OpSemantics::side_effect);
  op.external = flow::external_op("swapchain", "present", 1);
  op.argument_types.push_back(flow::type_id<rhi::Queue>());
  op.argument_types.push_back(
      flow::type_id<::strobe::rdg::swapchain_image_runtime>());
  op.argument_accesses.push_back(flow::AccessMode::read);
  op.argument_accesses.push_back(flow::AccessMode::consume);
  op.argument_groups.push_back({0, 1, 0, 1});
  op.argument_groups.push_back({1, 1, 1, 1});
  op.argument_elements.push_back(0);
  op.argument_elements.push_back(1);
  return op;
}
struct PresentMarker {
  flow::detail::NodeId node;
  flow::ResourceId image;
  flow::ResourceId resource;
  flow::ResourceId command_buffer;
  flow::ResourceId wait_command_buffer;
  flow::detail::compiler::SourceOperationId position;
  flow::detail::compiler::SourceOperationId submit_position;
  flow::detail::compiler::SourceOperationId boundary;
};
flow::detail::compiler::SourceOperationId
presentation_boundary(const flow::detail::compiler::NormalizedIR &ir,
                      flow::detail::compiler::SourceOperationId position) {
  auto boundary = flow::detail::compiler::null_source_operation_id;
  for (const auto &phi : ir.phis) {
    if (phi.declaration_position <= position &&
        position < phi.feedback_position && phi.feedback_position < boundary) {
      boundary = phi.feedback_position;
    }
  }
  return boundary;
}
void lower_swapchain(flow::detail::compiler::NormalizedIR &ir,
                     flow::CompileContext &context) {
  const auto &state = compiler_state(context);
  const auto queue = state.queue;
  const auto &submits = state.submissions;
  const auto &image_uses = state.image_uses;
  using namespace flow::detail;
  using namespace flow::detail::compiler;

  Vector<PresentMarker, flow::allocator_ref> presents;
  for (NodeId node_id = 0; node_id < ir.nodes.size(); ++node_id) {
    const auto &node = ir.nodes[node_id];
    if (!intrinsic(node, "present")) {
      continue;
    }
    const auto accesses = ir.accesses_of(node);
    if (queue == flow::null_resource_id || node.kind != NodeKind::callback ||
        accesses.size() != 3 || node.argument_count != 2) {
      throw std::logic_error("present intrinsic contract is invalid");
    }
    const auto image = accesses[0];
    const auto resource = accesses[1];
    const auto command_buffer = accesses[2];
    if (image.resource >= ir.resources.size() ||
        ir.resources[image.resource].type !=
            flow::type_id<::strobe::rdg::swapchain_image_runtime>() ||
        image.mode != flow::AccessMode::consume ||
        image.purpose != flow::AccessPurpose::argument ||
        resource.resource >= ir.resources.size() ||
        ir.resources[resource.resource].type != flow::type_id<rhi::Image>() ||
        resource.mode != flow::AccessMode::read ||
        resource.purpose != flow::AccessPurpose::argument ||
        command_buffer.resource >= ir.resources.size() ||
        ir.resources[command_buffer.resource].type !=
            flow::type_id<rhi::CommandBuffer>() ||
        command_buffer.mode != flow::AccessMode::read_write ||
        command_buffer.purpose != flow::AccessPurpose::serialization) {
      throw std::logic_error("present intrinsic accesses are invalid");
    }

    const auto boundary = presentation_boundary(ir, node.source_operation);
    const auto submit =
        std::ranges::find_if(submits, [&](const auto &candidate) {
          return candidate.command_buffer == command_buffer.resource &&
                 node.source_operation < candidate.position &&
                 (boundary == null_source_operation_id ||
                  candidate.position < boundary);
        });
    if (submit == submits.end()) {
      throw std::logic_error(
          "present intrinsic has no following command-buffer submission");
    }
    const ImageUse *first_use = nullptr;
    for (const auto &use : image_uses) {
      if (use.image == resource.resource &&
          (first_use == nullptr ||
           use.submit_position < first_use->submit_position)) {
        first_use = &use;
      }
    }
    if (first_use == nullptr) {
      throw std::logic_error("presented image has no device access");
    }
    presents.push_back(PresentMarker{
        .node = node_id,
        .image = image.resource,
        .resource = resource.resource,
        .command_buffer = command_buffer.resource,
        .wait_command_buffer = first_use->command_buffer,
        .position = node.source_operation,
        .submit_position = first_use->submit_position,
        .boundary = boundary,
    });
  }

  if (presents.empty()) {
    return;
  }

  std::ranges::stable_sort(presents, {}, &PresentMarker::boundary);
  Vector<NormalizedNode, flow::allocator_ref> old_nodes = std::move(ir.nodes);
  SourceOperationId end_position = 0;
  for (const auto &node : old_nodes) {
    if (node.source_operation != null_source_operation_id) {
      end_position = std::max(end_position, node.source_operation + 1);
    }
  }
  Vector<NodeId, flow::allocator_ref> remap(old_nodes.size(), null_node_id,
                                            flow::allocator_ref{});
  ir.nodes.reserve(old_nodes.size() + presents.size());

  const auto append_wait = [&](const PresentMarker &marker) {
    const auto offset = ir.accesses.size();
    ir.accesses.push_back(
        {queue, flow::AccessMode::read, flow::AccessPurpose::argument});
    ir.accesses.push_back(
        {marker.image, flow::AccessMode::read, flow::AccessPurpose::argument});
    ir.accesses.push_back({marker.wait_command_buffer,
                           flow::AccessMode::read_write,
                           flow::AccessPurpose::serialization});
    ir.accesses.push_back({queue, flow::AccessMode::read_write,
                           flow::AccessPurpose::serialization});
    const auto group_offset = ir.argument_groups.size();
    ir.argument_groups.push_back({0, 1, 0, 1});
    ir.argument_groups.push_back({1, 1, 1, 1});
    const auto element_offset = ir.argument_elements.size();
    ir.argument_elements.push_back(0);
    ir.argument_elements.push_back(1);
    const auto node = static_cast<NodeId>(ir.nodes.size());
    ir.nodes.push_back(NormalizedNode{
        .kind = NodeKind::callback,
        .source_operation = marker.submit_position,
        .accesses = {.offset = static_cast<std::uint32_t>(offset), .count = 4},
        .argument_groups = {
            .offset = static_cast<std::uint32_t>(group_offset), .count = 2},
        .argument_elements = {
            .offset = static_cast<std::uint32_t>(element_offset), .count = 2},
        .argument_count = 2,
        .resolved_operation = context.resolve_operation(wait_swapchain_op()),
        .semantics = flow::OpSemantics::side_effect,
        .intrinsic = std::nullopt,
    });
    return node;
  };

  const auto append_deferred_present = [&](const PresentMarker &marker) {
    const auto offset = ir.accesses.size();
    ir.accesses.push_back(
        {queue, flow::AccessMode::read, flow::AccessPurpose::argument});
    ir.accesses.push_back({marker.image, flow::AccessMode::consume,
                           flow::AccessPurpose::argument});
    ir.accesses.push_back({marker.command_buffer, flow::AccessMode::read_write,
                           flow::AccessPurpose::serialization});
    ir.accesses.push_back({queue, flow::AccessMode::read_write,
                           flow::AccessPurpose::serialization});
    const auto group_offset = ir.argument_groups.size();
    ir.argument_groups.push_back({0, 1, 0, 1});
    ir.argument_groups.push_back({1, 1, 1, 1});
    const auto element_offset = ir.argument_elements.size();
    ir.argument_elements.push_back(0);
    ir.argument_elements.push_back(1);
    const auto source_operation = marker.boundary == null_source_operation_id
                                      ? end_position
                                      : marker.boundary - 1;
    ir.nodes.push_back(NormalizedNode{
        .kind = NodeKind::callback,
        .source_operation = source_operation,
        .accesses = {.offset = static_cast<std::uint32_t>(offset), .count = 4},
        .argument_groups = {
            .offset = static_cast<std::uint32_t>(group_offset), .count = 2},
        .argument_elements = {
            .offset = static_cast<std::uint32_t>(element_offset), .count = 2},
        .argument_count = 2,
        .resolved_operation = context.resolve_operation(present_swapchain_op()),
        .semantics = flow::OpSemantics::side_effect,
        .intrinsic = std::nullopt,
    });
  };

  std::size_t deferred = 0;
  for (NodeId old_id = 0; old_id < old_nodes.size(); ++old_id) {
    auto node = std::move(old_nodes[old_id]);
    while (deferred < presents.size() &&
           presents[deferred].boundary != null_source_operation_id &&
           node.source_operation != null_source_operation_id &&
           presents[deferred].boundary <= node.source_operation) {
      append_deferred_present(presents[deferred++]);
    }

    const auto marker = std::ranges::find_if(
        presents, [&](const auto &present) { return present.node == old_id; });
    if (marker != presents.end()) {
      continue;
    }

    for (const auto &present : presents) {
      if (present.submit_position == node.source_operation) {
        remap[present.node] = append_wait(present);
      }
    }
    remap[old_id] = static_cast<NodeId>(ir.nodes.size());
    ir.nodes.push_back(std::move(node));
  }
  while (deferred < presents.size()) {
    append_deferred_present(presents[deferred++]);
  }

  for (auto &phi : ir.phis) {
    phi.node = remap[phi.node];
  }
  for (auto &cutoff : ir.cutoffs) {
    cutoff.node = remap[cutoff.node];
  }
}

} // namespace strobe::rdg::detail
