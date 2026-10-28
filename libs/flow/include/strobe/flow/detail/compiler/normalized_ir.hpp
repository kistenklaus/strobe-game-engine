#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/access_purpose.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/argument_group.hpp"
#include "strobe/flow/program.hpp"
#include "strobe/flow/projection.hpp"
#include "strobe/flow/resource_id.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>

namespace strobe::flow::detail::compiler {

using SourceOperationId = std::uint32_t;

inline constexpr SourceOperationId null_source_operation_id =
    std::numeric_limits<SourceOperationId>::max();

enum class ResourceKind : std::uint8_t {
  owned,
  external,
  phi,
  cutoff,
  projection,
};

enum class NodeKind : std::uint8_t {
  callback,
  construct,
  cutoff,
  discard,
  acquire,
  yield,
  phi,
};

struct NormalizedResource {
  ResourceKind kind;
  TypeId type;
  String<allocator_ref> type_name;
  String<allocator_ref> label;
  std::size_t size;
  std::size_t alignment;
  void (*destroy)(void *) noexcept;
  std::uint32_t detail;
};

struct NormalizedAccess {
  ResourceId resource;
  AccessMode mode;
  AccessPurpose purpose = AccessPurpose::argument;
};

struct NormalizedNode {
  NodeKind kind;
  SourceOperationId source_operation;
  IndexRange accesses;
  IndexRange argument_groups;
  IndexRange argument_elements;
  std::uint32_t argument_count = 0;
  ResolvedOperationId resolved_operation = null_resolved_operation_id;
  OpSemantics semantics = OpSemantics::none;
  std::optional<IntrinsicOp> intrinsic;
};

struct NormalizedPhi {
  NodeId node;
  ResourceId output;
  ResourceId initial;
  ResourceId feedback;
  std::optional<ResourceId> condition;
  SourceOperationId declaration_position;
  SourceOperationId feedback_position;
  std::uint32_t max_in_flight;
};

struct NormalizedCutoff {
  NodeId node;
  ResourceId input;
  ResourceId output;
  std::uint32_t source_cutoff;
};

struct NormalizedExternal {
  ResourceId resource;
  std::uint32_t source_external;
};

struct NormalizedProjection {
  ResourceId resource;
  ResourceId owner;
  ProjectionKind kind;
  std::uint32_t source_projection;
};

struct NormalizedIf {
  ResourceId condition;
  SourceOperationId begin;
  SourceOperationId alternative;
  SourceOperationId end;
};

struct NormalizedIR {
  Vector<NormalizedResource, allocator_ref> resources;
  Vector<NormalizedNode, allocator_ref> nodes;
  Vector<NormalizedAccess, allocator_ref> accesses;
  Vector<ArgumentGroup, allocator_ref> argument_groups;
  Vector<std::uint32_t, allocator_ref> argument_elements;
  Vector<NormalizedPhi, allocator_ref> phis;
  Vector<NormalizedCutoff, allocator_ref> cutoffs;
  Vector<NormalizedExternal, allocator_ref> externals;
  Vector<NormalizedProjection, allocator_ref> projections;
  Vector<NormalizedIf, allocator_ref> ifs;

  span<NormalizedAccess> accesses_of(NormalizedNode &node) noexcept;
  span<const NormalizedAccess>
  accesses_of(const NormalizedNode &node) const noexcept;
  span<NormalizedAccess> accesses_of(NodeId node) noexcept;
  span<const NormalizedAccess> accesses_of(NodeId node) const noexcept;

  span<ArgumentGroup> argument_groups_of(NormalizedNode &node) noexcept;
  span<const ArgumentGroup>
  argument_groups_of(const NormalizedNode &node) const noexcept;
  span<ArgumentGroup> argument_groups_of(NodeId node) noexcept;
  span<const ArgumentGroup> argument_groups_of(NodeId node) const noexcept;

  span<std::uint32_t> argument_elements_of(NormalizedNode &node) noexcept;
  span<const std::uint32_t>
  argument_elements_of(const NormalizedNode &node) const noexcept;
  span<std::uint32_t> argument_elements_of(NodeId node) noexcept;
  span<const std::uint32_t> argument_elements_of(NodeId node) const noexcept;
};

String<allocator_ref> to_pretty(const NormalizedIR &ir);

} // namespace strobe::flow::detail::compiler
