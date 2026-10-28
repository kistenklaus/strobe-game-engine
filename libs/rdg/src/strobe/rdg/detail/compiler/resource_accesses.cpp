#include "compiler.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/image.hpp"

#include <stdexcept>

namespace strobe::rdg::detail {

void collect_resource_accesses(flow::detail::compiler::NormalizedIR &ir,
                               flow::CompileContext &context) {
  auto &state = compiler_state(context);
  state.removed_markers = Vector<std::uint8_t, flow::allocator_ref>(
      ir.nodes.size(), std::uint8_t{0}, flow::allocator_ref{});

  for (flow::detail::NodeId node_id = 0; node_id < ir.nodes.size(); ++node_id) {
    const auto &node = ir.nodes[node_id];
    const auto accesses = ir.accesses_of(node);
    if (intrinsic(node, "image-access")) {
      if (accesses.size() != 3 || node.argument_count != 2) {
        throw std::logic_error("image-access intrinsic contract is invalid");
      }
      const auto command = accesses[0];
      const auto image = accesses[1];
      const auto pool = accesses[2];
      if (command.resource >= ir.resources.size() ||
          ir.resources[command.resource].type !=
              flow::type_id<rhi::CommandBuffer>() ||
          command.mode != flow::AccessMode::read_write ||
          command.purpose != flow::AccessPurpose::argument ||
          image.resource >= ir.resources.size() ||
          ir.resources[image.resource].type != flow::type_id<rhi::Image>() ||
          image.mode != flow::AccessMode::read ||
          image.purpose != flow::AccessPurpose::argument ||
          pool.purpose != flow::AccessPurpose::serialization) {
        throw std::logic_error("image-access intrinsic accesses are invalid");
      }
      const auto &operation = *node.intrinsic;
      state.image_markers.push_back(ImageMarker{
          .kind = ImageMarkerKind::access,
          .node = node_id,
          .command_buffer = command.resource,
          .image = image.resource,
          .position = node.source_operation,
          .submit_position = flow::detail::compiler::null_source_operation_id,
          .state =
              {
                  .stage = operation.immediate<rhi::PipelineStage>(0),
                  .access = operation.immediate<rhi::Access>(1),
                  .layout = operation.immediate<rhi::ImageLayout>(2),
              },
      });
      continue;
    }
    if (intrinsic(node, "cmd-discard-image")) {
      if (accesses.size() != 3 || node.argument_count != 2) {
        throw std::logic_error(
            "cmd-discard-image intrinsic contract is invalid");
      }
      const auto command = accesses[0];
      const auto image = accesses[1];
      const auto pool = accesses[2];
      if (command.resource >= ir.resources.size() ||
          ir.resources[command.resource].type !=
              flow::type_id<rhi::CommandBuffer>() ||
          command.mode != flow::AccessMode::read_write ||
          command.purpose != flow::AccessPurpose::argument ||
          image.resource >= ir.resources.size() ||
          ir.resources[image.resource].type != flow::type_id<rhi::Image>() ||
          image.mode != flow::AccessMode::read_write ||
          image.purpose != flow::AccessPurpose::argument ||
          pool.purpose != flow::AccessPurpose::serialization) {
        throw std::logic_error(
            "cmd-discard-image intrinsic accesses are invalid");
      }
      state.image_markers.push_back(ImageMarker{
          .kind = ImageMarkerKind::discard,
          .node = node_id,
          .command_buffer = command.resource,
          .image = image.resource,
          .position = node.source_operation,
          .submit_position = flow::detail::compiler::null_source_operation_id,
          .state = {},
      });
      continue;
    }
    if (intrinsic(node, "buffer-access") ||
        intrinsic(node, "cmd-buffer-access")) {
      if (accesses.size() != 3 || node.argument_count != 2) {
        throw std::logic_error("buffer-access intrinsic contract is invalid");
      }
      const auto command = accesses[0];
      const auto buffer = accesses[1];
      const auto pool = accesses[2];
      if (command.resource >= ir.resources.size() ||
          ir.resources[command.resource].type !=
              flow::type_id<rhi::CommandBuffer>() ||
          command.mode != flow::AccessMode::read_write ||
          buffer.resource >= ir.resources.size() ||
          ir.resources[buffer.resource].type != flow::type_id<rhi::Buffer>() ||
          buffer.mode != flow::AccessMode::read ||
          pool.purpose != flow::AccessPurpose::serialization) {
        throw std::logic_error("buffer-access intrinsic accesses are invalid");
      }
      const auto &operation = *node.intrinsic;
      state.buffer_accesses.push_back(BufferAccessMarker{
          .node = node_id,
          .command_buffer = command.resource,
          .buffer = buffer.resource,
          .position = node.source_operation,
          .submit_position = flow::detail::compiler::null_source_operation_id,
          .state =
              {
                  .stage = operation.immediate<rhi::PipelineStage>(0),
                  .access = operation.immediate<rhi::Access>(1),
              },
      });
    }
  }
}

} // namespace strobe::rdg::detail
