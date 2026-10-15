#pragma once

#include "strobe/flow/detail/compiler/classified_ir.hpp"

#include <strobe/core/graph.hpp>

#include <cstdint>
#include <utility>

namespace strobe::flow::detail::compiler {

enum class DependencyKind : std::uint8_t {
  value,
  resource_order,
  temporal,
};

struct DependencyNode {
  NodeId source;
  NodeKind kind;
  NodeClassification classification;
};

struct DependencyEdge {
  DependencyKind kind;
  ValueId value;
  ResourceId resource;
};

using CompactDependencyGraph =
    graph::ConstGraph<DependencyNode, DependencyEdge, allocator_ref>;

struct GraphIR {
  GraphIR(ClassifiedIR classified, CompactDependencyGraph graph)
      : classified(std::move(classified)), graph(std::move(graph)) {}

  ClassifiedIR classified;
  CompactDependencyGraph graph;
};

} // namespace strobe::flow::detail::compiler
