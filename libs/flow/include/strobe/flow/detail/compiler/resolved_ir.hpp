#pragma once

#include "strobe/flow/detail/compiler/value_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>

namespace strobe::flow::detail::compiler {

enum class PhiEdgeKind : std::uint8_t {
  initial,
  feedback,
};

struct PhiEdge {
  ValueId value;
  NodeId phi;
  PhiEdgeKind kind;
};

struct Backedge {
  ValueId value;
  ValueId condition;
  NodeId target;
};

struct ResolvedIR {
  ValueIR values;
  Vector<PhiEdge, allocator_ref> phi_edges;
  Vector<Backedge, allocator_ref> backedges;
};

} // namespace strobe::flow::detail::compiler
