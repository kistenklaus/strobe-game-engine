#pragma once

#include "strobe/core/graph/const_graph.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/memory.hpp>

#include <cstddef>
#include <cstdint>

namespace strobe::graph {

/**
 * \brief Contains a topological order and its cycle status.
 * \code{.cpp}
 * template<Allocator A = Mallocator>
 * struct TopologicalSortResult;
 * \endcode
 */
template <Allocator A = Mallocator> struct TopologicalSortResult {
  Vector<NodeId, A> order;
  bool acyclic;
};

/**
 * \brief Computes a topological order with Kahn's algorithm.
 * \code{.cpp}
 * template<typename N, typename E, Allocator GA, Allocator A = Mallocator>
 * TopologicalSortResult<A>
 * topological_sort(const ConstGraph<N, E, GA>& graph,
 *                  const A& allocator = {});
 * \endcode
 * \param graph Directed graph to sort.
 * \param allocator Allocator used by the result and temporary storage.
 * \return The complete order for an acyclic graph, or the maximal removable
 * prefix and `acyclic == false` when the graph contains a cycle.
 */
template <typename N, typename E, Allocator GA, Allocator A = Mallocator>
TopologicalSortResult<A>
topological_sort(const ConstGraph<N, E, GA> &graph,
                 const A &allocator = {}) {
  Vector<std::size_t, A> indegree(graph.node_count(), 0, allocator);
  Vector<NodeId, A> ready(allocator);
  Vector<NodeId, A> order(allocator);
  ready.reserve(graph.node_count());
  order.reserve(graph.node_count());

  for (std::uint32_t node = 0; node < graph.node_count(); ++node) {
    indegree[node] = graph.incoming(NodeId{node}).size();
    if (indegree[node] == 0) {
      ready.push_back(NodeId{node});
    }
  }

  std::size_t head = 0;
  while (head < ready.size()) {
    const auto node = ready[head++];
    order.push_back(node);
    for (const auto edge : graph.outgoing(node)) {
      const auto destination = graph.destination(edge);
      if (--indegree[destination.value] == 0) {
        ready.push_back(destination);
      }
    }
  }

  const bool acyclic = order.size() == graph.node_count();
  return {std::move(order), acyclic};
}

} // namespace strobe::graph
