#pragma once

#include "strobe/flow/detail/compiler/normalized_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>

namespace strobe::flow::detail::compiler {

enum class StaticValueFlags : std::uint8_t {
  none = 0,
  external = 1,
};

struct StaticValue {
  ResourceId resource;
  NodeId producer;
  std::uint32_t version;
  StaticValueFlags flags;
};

struct ValueAccess {
  ResourceId resource;
  AccessMode mode;
  ValueId input;
  ValueId output;
};

struct ValueNode {
  NodeKind kind;
  SourceOperationId source_operation;
  IndexRange accesses;
  OpSemantics semantics = OpSemantics::none;
};

struct ValuePhi {
  NodeId node;
  ResourceId feedback_resource;
  ValueId output;
  ValueId initial;
  ValueId feedback;
  std::optional<ResourceId> condition_resource;
  ValueId condition;
  SourceOperationId declaration_position;
  SourceOperationId feedback_position;
};

struct ValueIf {
  ResourceId condition_resource;
  ValueId condition;
  SourceOperationId begin;
  SourceOperationId alternative;
  SourceOperationId end;
};

struct ValueIR {
  Vector<NormalizedResource, allocator_ref> resources;
  Vector<ValueNode, allocator_ref> nodes;
  Vector<ValueAccess, allocator_ref> accesses;
  Vector<StaticValue, allocator_ref> values;
  Vector<ValuePhi, allocator_ref> phis;
  Vector<NormalizedCutoff, allocator_ref> cutoffs;
  Vector<NormalizedExternal, allocator_ref> externals;
  Vector<ValueIf, allocator_ref> ifs;
};

} // namespace strobe::flow::detail::compiler
