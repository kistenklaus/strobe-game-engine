#pragma once

#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/detail/compiler/normalized_ir.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace strobe::rdg::detail {

struct SubmitMarker {
  flow::ResourceId command_buffer;
  flow::detail::compiler::SourceOperationId position;
};

struct ImageUse {
  flow::ResourceId image;
  flow::ResourceId command_buffer;
  flow::detail::compiler::SourceOperationId position;
  flow::detail::compiler::SourceOperationId submit_position;
};

struct ImageState {
  rhi::PipelineStage stage;
  rhi::Access access;
  rhi::ImageLayout layout;
};

struct BufferState {
  rhi::PipelineStage stage;
  rhi::Access access;
};

enum class ImageMarkerKind : std::uint8_t {
  access,
  discard,
};

struct ImageMarker {
  ImageMarkerKind kind;
  flow::detail::NodeId node;
  flow::ResourceId command_buffer;
  flow::ResourceId image;
  flow::detail::compiler::SourceOperationId position;
  flow::detail::compiler::SourceOperationId submit_position;
  ImageState state;
};

struct BufferAccessMarker {
  flow::detail::NodeId node;
  flow::ResourceId command_buffer;
  flow::ResourceId buffer;
  flow::detail::compiler::SourceOperationId position;
  flow::detail::compiler::SourceOperationId submit_position;
  BufferState state;
};

struct CompilerState {
  flow::ResourceId device = flow::null_resource_id;
  flow::ResourceId queue = flow::null_resource_id;
  Vector<SubmitMarker, flow::allocator_ref> submissions;
  Vector<ImageUse, flow::allocator_ref> image_uses;
  Vector<std::optional<ImageState>, flow::allocator_ref> image_states;
  Vector<std::optional<BufferState>, flow::allocator_ref> buffer_states;
  Vector<ImageMarker, flow::allocator_ref> image_markers;
  Vector<BufferAccessMarker, flow::allocator_ref> buffer_accesses;
  Vector<std::uint8_t, flow::allocator_ref> removed_markers;
};

inline constexpr std::string_view compiler_state_id = "strobe.rdg.compiler";

bool intrinsic(const flow::detail::compiler::NormalizedNode &node,
               std::string_view opcode);

flow::ResourceId
single_resource(const flow::detail::compiler::NormalizedIR &ir,
                const flow::detail::compiler::NormalizedNode &node,
                flow::TypeId type, flow::AccessMode mode);

CompilerState &compiler_state(flow::CompileContext &context);

flow::detail::compiler::SourceOperationId
submission_for(const Vector<SubmitMarker, flow::allocator_ref> &submissions,
               flow::ResourceId command,
               flow::detail::compiler::SourceOperationId access_position);

void erase_marked_nodes(
    flow::detail::compiler::NormalizedIR &ir,
    const Vector<std::uint8_t, flow::allocator_ref> &removed);

template <typename Materializer>
void materialize(flow::detail::compiler::NormalizedIR &ir,
                 flow::CompileContext &context, std::string_view opcode,
                 Materializer &&materializer) {
  for (auto &node : ir.nodes) {
    if (!intrinsic(node, opcode)) {
      continue;
    }
    materializer(ir, context, node);
    if (node.intrinsic) {
      throw std::logic_error("RDG intrinsic materializer did not consume node");
    }
  }
}

} // namespace strobe::rdg::detail
