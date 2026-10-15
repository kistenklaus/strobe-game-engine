#pragma once

#include "strobe/core/graph/const_graph.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>
#include <strobe/memory.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace strobe::graph {

/**
 * \brief Dense identifier of a strongly connected component.
 * \code{.cpp}
 * using ComponentId = std::uint32_t;
 * \endcode
 */
using ComponentId = std::uint32_t;

/**
 * \brief Stores a graph partition into strongly connected components.
 * \code{.cpp}
 * template<Allocator A = Mallocator>
 * class StronglyConnectedComponents;
 * \endcode
 */
template <Allocator A = Mallocator> class StronglyConnectedComponents {
public:
  /**
   * \brief Constructs a component partition from compact arrays.
   * \code{.cpp}
   * StronglyConnectedComponents(Vector<ComponentId, A> component_of,
   *                             Vector<NodeId, A> nodes,
   *                             Vector<std::size_t, A> offsets);
   * \endcode
   * \param component_of Component identifier for every graph node.
   * \param nodes Nodes grouped by component.
   * \param offsets Component boundaries in `nodes`.
   * \attention 1. `offsets` must contain an initial zero and one terminal
   * boundary for every component.
   * \attention 2. The arrays must describe one complete, consistent graph
   * partition.
   */
  StronglyConnectedComponents(Vector<ComponentId, A> component_of,
                              Vector<NodeId, A> nodes,
                              Vector<std::size_t, A> offsets)
      : m_component_of(std::move(component_of)), m_nodes(std::move(nodes)),
        m_offsets(std::move(offsets)) {}

  /**
   * \brief Returns the number of components.
   * \code{.cpp}
   * std::size_t component_count() const noexcept;
   * \endcode
   * \return Number of strongly connected components.
   */
  std::size_t component_count() const noexcept { return m_offsets.size() - 1; }

  /**
   * \brief Returns the component containing a node.
   * \code{.cpp}
   * ComponentId component_of(NodeId node) const noexcept;
   * \endcode
   * \param node Dense graph node identifier.
   * \return Component containing the node.
   * \attention 1. The node identifier must be in range.
   */
  ComponentId component_of(NodeId node) const noexcept {
    return m_component_of[node.value];
  }

  /**
   * \brief Returns the nodes belonging to a component.
   * \code{.cpp}
   * span<const NodeId> component(ComponentId component) const noexcept;
   * \endcode
   * \param component Dense component identifier.
   * \return Contiguous nodes belonging to the component.
   * \attention 1. The component identifier must be in range.
   */
  span<const NodeId> component(ComponentId component) const noexcept {
    const auto begin = m_offsets[component];
    const auto count = m_offsets[component + 1] - begin;
    return {m_nodes.data() + begin, count};
  }

private:
  Vector<ComponentId, A> m_component_of;
  Vector<NodeId, A> m_nodes;
  Vector<std::size_t, A> m_offsets;
};

/**
 * \brief Partitions a directed graph into strongly connected components.
 * \code{.cpp}
 * template<typename N, typename E, Allocator GA, Allocator A = Mallocator>
 * StronglyConnectedComponents<A>
 * strongly_connected_components(const ConstGraph<N, E, GA>& graph,
 *                               const A& allocator = {});
 * \endcode
 * \param graph Directed graph to partition.
 * \param allocator Allocator used by the result and temporary storage.
 * \return Dense strongly connected component partition.
 */
template <typename N, typename E, Allocator GA, Allocator A = Mallocator>
StronglyConnectedComponents<A>
strongly_connected_components(const ConstGraph<N, E, GA> &graph,
                              const A &allocator = {}) {
  struct Frame {
    NodeId node;
    std::size_t next_edge;
  };

  Vector<std::uint8_t, A> visited(graph.node_count(), 0, allocator);
  Vector<NodeId, A> finish(allocator);
  Vector<Frame, A> frames(allocator);
  finish.reserve(graph.node_count());
  frames.reserve(graph.node_count());

  for (std::uint32_t start = 0; start < graph.node_count(); ++start) {
    if (visited[start]) {
      continue;
    }
    visited[start] = 1;
    frames.push_back(Frame{NodeId{start}, 0});
    while (!frames.empty()) {
      auto &frame = frames.back();
      const auto outgoing = graph.outgoing(frame.node);
      if (frame.next_edge < outgoing.size()) {
        const auto next = graph.destination(outgoing[frame.next_edge++]);
        if (!visited[next.value]) {
          visited[next.value] = 1;
          frames.push_back(Frame{next, 0});
        }
        continue;
      }
      finish.push_back(frame.node);
      frames.pop_back();
    }
  }

  constexpr auto unassigned = std::numeric_limits<ComponentId>::max();
  Vector<ComponentId, A> component_of(graph.node_count(), unassigned,
                                      allocator);
  Vector<NodeId, A> nodes(allocator);
  Vector<std::size_t, A> offsets(allocator);
  Vector<NodeId, A> stack(allocator);
  nodes.reserve(graph.node_count());
  offsets.push_back(0);

  while (!finish.empty()) {
    const auto start = finish.back();
    finish.pop_back();
    if (component_of[start.value] != unassigned) {
      continue;
    }
    const auto component = static_cast<ComponentId>(offsets.size() - 1);
    component_of[start.value] = component;
    stack.push_back(start);
    while (!stack.empty()) {
      const auto node = stack.back();
      stack.pop_back();
      nodes.push_back(node);
      for (const auto edge : graph.incoming(node)) {
        const auto next = graph.source(edge);
        if (component_of[next.value] == unassigned) {
          component_of[next.value] = component;
          stack.push_back(next);
        }
      }
    }
    offsets.push_back(nodes.size());
  }

  return StronglyConnectedComponents<A>(std::move(component_of),
                                         std::move(nodes),
                                         std::move(offsets));
}

} // namespace strobe::graph
