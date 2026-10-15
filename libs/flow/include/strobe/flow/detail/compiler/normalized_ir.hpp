#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/program.hpp"
#include "strobe/flow/resource_id.hpp"

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
  std::size_t size;
  std::size_t alignment;
  void (*destroy)(void *) noexcept;
  std::uint32_t detail;
};

struct NormalizedAccess {
  ResourceId resource;
  AccessMode mode;
};

struct NormalizedNode {
  NodeKind kind;
  SourceOperationId source_operation;
  IndexRange accesses;
  OpSemantics semantics = OpSemantics::none;
};

struct NormalizedPhi {
  NodeId node;
  ResourceId output;
  ResourceId initial;
  ResourceId feedback;
  std::optional<ResourceId> condition;
  SourceOperationId declaration_position;
  SourceOperationId feedback_position;
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
  Vector<NormalizedPhi, allocator_ref> phis;
  Vector<NormalizedCutoff, allocator_ref> cutoffs;
  Vector<NormalizedExternal, allocator_ref> externals;
  Vector<NormalizedIf, allocator_ref> ifs;
};

} // namespace strobe::flow::detail::compiler
