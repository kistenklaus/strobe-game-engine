#pragma once

#include "strobe/flow/detail/compiler/ordered_ir.hpp"

#include <strobe/core/containers/vector.hpp>

namespace strobe::flow::detail::compiler {

struct NodeClassification {
  bool root;
  bool reads_external;
  bool writes_external;
  bool observable_effect;
  bool loop_invariant = false;
};

struct ClassifiedIR {
  OrderedIR ordered;
  Vector<NodeClassification, allocator_ref> nodes;
  Vector<bool, allocator_ref> preconstructed_resources;
  Vector<NodeId, allocator_ref> roots;
  Vector<NodeId, allocator_ref> observable_effects;
};

} // namespace strobe::flow::detail::compiler
