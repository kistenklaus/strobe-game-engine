#pragma once

#include "strobe/flow/detail/compiler/lifetime_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <limits>
#include <utility>

namespace strobe::flow::detail::compiler {

inline constexpr graph::NodeId null_interference_node{
    std::numeric_limits<std::uint32_t>::max()};

struct InterferenceResource {
  ResourceId resource;
  std::size_t size;
  std::size_t alignment;
  StorageClass storage;
  bool may_overlap_generations;
};

using InterferenceGraph =
    graph::ConstGraph<InterferenceResource, graph::EmptyEdge, allocator_ref>;

struct InterferenceIR {
  InterferenceIR(LifetimeIR input, InterferenceGraph graph,
                 Vector<graph::NodeId, allocator_ref> resource_to_node)
      : input(std::move(input)), graph(std::move(graph)),
        resource_to_node(std::move(resource_to_node)) {}

  LifetimeIR input;
  InterferenceGraph graph;
  Vector<graph::NodeId, allocator_ref> resource_to_node;
};

} // namespace strobe::flow::detail::compiler
