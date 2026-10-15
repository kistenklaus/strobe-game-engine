#pragma once

#include "strobe/core/graph/adj_graph.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>
#include <strobe/memory.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace strobe::graph {

/**
 * \brief Immutable directed graph with dense identifiers and compact adjacency.
 * \code{.cpp}
 * template<typename N, typename E = EmptyEdge, Allocator A = Mallocator>
 * class ConstGraph;
 * \endcode
 */
template <typename N, typename E = EmptyEdge, Allocator A = Mallocator>
class ConstGraph {
public:
  /**
   * \brief Compacts a mutable graph by copying its values.
   * \code{.cpp}
   * template<Allocator GA>
   * explicit ConstGraph(const AdjGraph<N, E, GA>& graph,
   *                     const A& allocator = {});
   * \endcode
   * \param graph Mutable graph to compact.
   * \param allocator Allocator used by compact graph storage.
   */
  template <Allocator GA>
  explicit ConstGraph(const AdjGraph<N, E, GA> &graph,
                      const A &allocator = {})
      : m_nodes(allocator), m_edges(allocator), m_node_values(allocator),
        m_edge_values(allocator), m_adjacency(allocator) {
    build<false>(graph, allocator);
  }

  /**
   * \brief Compacts a mutable graph by moving its values.
   * \code{.cpp}
   * template<Allocator GA>
   * explicit ConstGraph(AdjGraph<N, E, GA>&& graph,
   *                     const A& allocator = {});
   * \endcode
   * \param graph Mutable graph whose values are moved into the compact graph.
   * \param allocator Allocator used by compact graph storage.
   */
  template <Allocator GA>
  explicit ConstGraph(AdjGraph<N, E, GA> &&graph, const A &allocator = {})
      : m_nodes(allocator), m_edges(allocator), m_node_values(allocator),
        m_edge_values(allocator), m_adjacency(allocator) {
    build<true>(graph, allocator);
  }

  /**
   * \brief Returns a node value.
   * \code{.cpp}
   * const N& get(NodeId node) const;
   * \endcode
   * \param node Dense node identifier.
   * \return The node value.
   * \attention 1. The identifier must be smaller than `node_count()`.
   */
  const N &get(NodeId node) const { return m_node_values[node.value]; }

  /**
   * \brief Returns an edge value.
   * \code{.cpp}
   * const E& get(EdgeId edge) const;
   * \endcode
   * \param edge Dense edge identifier.
   * \return The edge value.
   * \attention 1. The identifier must be smaller than `edge_count()`.
   */
  const E &get(EdgeId edge) const { return m_edge_values[edge.value]; }

  /**
   * \brief Returns the source of an edge.
   * \code{.cpp}
   * NodeId source(EdgeId edge) const;
   * \endcode
   * \param edge Dense edge identifier.
   * \return Source node identifier.
   * \attention 1. The identifier must be smaller than `edge_count()`.
   */
  NodeId source(EdgeId edge) const { return m_edges[edge.value].source; }

  /**
   * \brief Returns the destination of an edge.
   * \code{.cpp}
   * NodeId destination(EdgeId edge) const;
   * \endcode
   * \param edge Dense edge identifier.
   * \return Destination node identifier.
   * \attention 1. The identifier must be smaller than `edge_count()`.
   */
  NodeId destination(EdgeId edge) const {
    return m_edges[edge.value].destination;
  }

  /**
   * \brief Returns the incoming edges of a node.
   * \code{.cpp}
   * span<const EdgeId> incoming(NodeId node) const noexcept;
   * \endcode
   * \param node Dense node identifier.
   * \return Contiguous incoming edge identifiers.
   * \attention 1. The identifier must be smaller than `node_count()`.
   */
  span<const EdgeId> incoming(NodeId node) const noexcept {
    const auto &record = m_nodes[node.value];
    if (record.incoming_count == 0) {
      return {};
    }
    return {m_adjacency.data() + record.incoming_offset,
            record.incoming_count};
  }

  /**
   * \brief Returns the outgoing edges of a node.
   * \code{.cpp}
   * span<const EdgeId> outgoing(NodeId node) const noexcept;
   * \endcode
   * \param node Dense node identifier.
   * \return Contiguous outgoing edge identifiers.
   * \attention 1. The identifier must be smaller than `node_count()`.
   */
  span<const EdgeId> outgoing(NodeId node) const noexcept {
    const auto &record = m_nodes[node.value];
    if (record.outgoing_count == 0) {
      return {};
    }
    return {m_adjacency.data() + record.outgoing_offset,
            record.outgoing_count};
  }

  /**
   * \brief Returns the number of nodes.
   * \code{.cpp}
   * std::size_t node_count() const noexcept;
   * \endcode
   * \return Number of nodes.
   */
  std::size_t node_count() const noexcept { return m_nodes.size(); }

  /**
   * \brief Returns the number of edges.
   * \code{.cpp}
   * std::size_t edge_count() const noexcept;
   * \endcode
   * \return Number of edges.
   */
  std::size_t edge_count() const noexcept { return m_edges.size(); }

private:
  struct NodeRecord {
    std::size_t incoming_offset;
    std::size_t incoming_count;
    std::size_t outgoing_offset;
    std::size_t outgoing_count;
  };

  struct EdgeRecord {
    NodeId source;
    NodeId destination;
  };

  template <bool Move, typename G>
  void build(G &graph, const A &allocator) {
    constexpr auto invalid = std::numeric_limits<std::uint32_t>::max();
    Vector<NodeId, A> remap(graph.node_capacity(), NodeId{invalid}, allocator);
    m_nodes.reserve(graph.node_count());
    m_node_values.reserve(graph.node_count());
    m_edges.reserve(graph.edge_count());
    m_edge_values.reserve(graph.edge_count());

    for (std::uint32_t old = 0; old < graph.node_capacity(); ++old) {
      const NodeId old_id{old};
      if (!graph.contains(old_id)) {
        continue;
      }
      const NodeId dense{static_cast<std::uint32_t>(m_nodes.size())};
      remap[old] = dense;
      m_nodes.push_back(NodeRecord{});
      if constexpr (Move) {
        m_node_values.push_back(std::move(graph.get(old_id)));
      } else {
        m_node_values.push_back(graph.get(old_id));
      }
    }

    Vector<std::size_t, A> incoming_count(m_nodes.size(), 0, allocator);
    Vector<std::size_t, A> outgoing_count(m_nodes.size(), 0, allocator);
    for (std::uint32_t old = 0; old < graph.edge_capacity(); ++old) {
      const EdgeId old_id{old};
      if (!graph.contains(old_id)) {
        continue;
      }
      const auto source = remap[graph.source(old_id).value];
      const auto destination = remap[graph.destination(old_id).value];
      if (source.value == invalid || destination.value == invalid) {
        throw std::logic_error("live edge refers to a removed node");
      }
      m_edges.push_back(EdgeRecord{source, destination});
      if constexpr (Move) {
        m_edge_values.push_back(std::move(graph.get(old_id)));
      } else {
        m_edge_values.push_back(graph.get(old_id));
      }
      ++outgoing_count[source.value];
      ++incoming_count[destination.value];
    }

    std::size_t offset = 0;
    for (std::uint32_t node = 0; node < m_nodes.size(); ++node) {
      m_nodes[node].incoming_offset = offset;
      m_nodes[node].incoming_count = incoming_count[node];
      offset += incoming_count[node];
    }
    for (std::uint32_t node = 0; node < m_nodes.size(); ++node) {
      m_nodes[node].outgoing_offset = offset;
      m_nodes[node].outgoing_count = outgoing_count[node];
      offset += outgoing_count[node];
    }
    m_adjacency.resize(offset);

    Vector<std::size_t, A> incoming_cursor(m_nodes.size(), 0, allocator);
    Vector<std::size_t, A> outgoing_cursor(m_nodes.size(), 0, allocator);
    for (std::uint32_t edge = 0; edge < m_edges.size(); ++edge) {
      const auto &record = m_edges[edge];
      auto &source = m_nodes[record.source.value];
      auto &destination = m_nodes[record.destination.value];
      m_adjacency[destination.incoming_offset +
                  incoming_cursor[record.destination.value]++] = EdgeId{edge};
      m_adjacency[source.outgoing_offset +
                  outgoing_cursor[record.source.value]++] = EdgeId{edge};
    }
  }

  Vector<NodeRecord, A> m_nodes;
  Vector<EdgeRecord, A> m_edges;
  Vector<N, A> m_node_values;
  Vector<E, A> m_edge_values;
  Vector<EdgeId, A> m_adjacency;
};

} // namespace strobe::graph
