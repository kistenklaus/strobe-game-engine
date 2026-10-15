#pragma once

#include "strobe/core/graph/types.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/memory.hpp>

#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

namespace strobe::graph {

/**
 * \brief Mutable directed graph with stable sparse identifiers.
 * \code{.cpp}
 * template<typename N, typename E = EmptyEdge, Allocator A = Mallocator>
 * class AdjGraph;
 * \endcode
 */
template <typename N, typename E = EmptyEdge, Allocator A = Mallocator>
class AdjGraph {
public:
  /**
   * \brief Stores one directed edge.
   * \code{.cpp}
   * struct Edge;
   * \endcode
   */
  struct Edge {
    NodeId source;
    NodeId destination;
    E value;
  };

  /**
   * \brief Constructs the graph with an allocator.
   * \code{.cpp}
   * explicit AdjGraph(const A& allocator = {});
   * \endcode
   * \param allocator Allocator used by graph storage.
   */
  explicit AdjGraph(const A &allocator = {})
      : m_nodes(allocator), m_node_free(allocator), m_edges(allocator),
        m_edge_free(allocator) {}

  /**
   * \brief Adds a node and returns its stable identifier.
   * \code{.cpp}
   * NodeId add_node(N value);
   * \endcode
   * \param value Node value.
   * \return Identifier of the added node.
   */
  NodeId add_node(N value) { return emplace_node(std::move(value)); }

  /**
   * \brief Constructs a node in the graph.
   * \code{.cpp}
   * template<typename... Args> NodeId emplace_node(Args&&... args);
   * \endcode
   * \param args Arguments used to construct the node value.
   * \return Identifier of the added node.
   */
  template <typename... Args> NodeId emplace_node(Args &&...args) {
    if (!m_node_free.empty()) {
      const auto id = m_node_free.back();
      m_node_free.pop_back();
      m_nodes[id.value].emplace(std::forward<Args>(args)...);
      return id;
    }
    ensure_id_space(m_nodes.size());
    const NodeId id{static_cast<std::uint32_t>(m_nodes.size())};
    m_nodes.emplace_back(std::in_place, std::forward<Args>(args)...);
    return id;
  }

  /**
   * \brief Removes a node and every incident edge.
   * \code{.cpp}
   * bool remove_node(NodeId node);
   * \endcode
   * \param node Node to remove.
   * \return Whether the node existed.
   */
  bool remove_node(NodeId node) {
    if (!contains(node)) {
      return false;
    }
    for (std::size_t index = 0; index < m_edges.size(); ++index) {
      if (m_edges[index] &&
          (m_edges[index]->source == node ||
           m_edges[index]->destination == node)) {
        remove_edge(EdgeId{static_cast<std::uint32_t>(index)});
      }
    }
    m_nodes[node.value].reset();
    m_node_free.push_back(node);
    return true;
  }

  /**
   * \brief Adds a directed edge.
   * \code{.cpp}
   * EdgeId add_edge(NodeId source, NodeId destination, E value = {});
   * \endcode
   * \param source Source node.
   * \param destination Destination node.
   * \param value Edge value.
   * \return Identifier of the added edge.
   * \attention 1. Both node identifiers must refer to live nodes.
   */
  EdgeId add_edge(NodeId source, NodeId destination, E value = {}) {
    if (!contains(source) || !contains(destination)) {
      throw std::invalid_argument("edge endpoint is not a live node");
    }
    if (!m_edge_free.empty()) {
      const auto id = m_edge_free.back();
      m_edge_free.pop_back();
      m_edges[id.value].emplace(
          Edge{source, destination, std::move(value)});
      return id;
    }
    ensure_id_space(m_edges.size());
    const EdgeId id{static_cast<std::uint32_t>(m_edges.size())};
    m_edges.emplace_back(
        std::in_place, Edge{source, destination, std::move(value)});
    return id;
  }

  /**
   * \brief Removes an edge.
   * \code{.cpp}
   * bool remove_edge(EdgeId edge);
   * \endcode
   * \param edge Edge to remove.
   * \return Whether the edge existed.
   */
  bool remove_edge(EdgeId edge) {
    if (!contains(edge)) {
      return false;
    }
    m_edges[edge.value].reset();
    m_edge_free.push_back(edge);
    return true;
  }

  /**
   * \brief Returns whether an identifier refers to a live node or edge.
   * \code{.cpp}
   * bool contains(NodeId node) const noexcept;
   * bool contains(EdgeId edge) const noexcept;
   * \endcode
   * \param node Node identifier.
   * \param edge Edge identifier.
   * \return Whether the identified object exists.
   */
  bool contains(NodeId node) const noexcept {
    return node.value < m_nodes.size() && m_nodes[node.value].has_value();
  }
  bool contains(EdgeId edge) const noexcept {
    return edge.value < m_edges.size() && m_edges[edge.value].has_value();
  }

  /**
   * \brief Returns a node value.
   * \code{.cpp}
   * N& get(NodeId node);
   * const N& get(NodeId node) const;
   * \endcode
   * \param node Live node identifier.
   * \return The node value.
   * \attention 1. The identifier must refer to a live node.
   */
  N &get(NodeId node) { return m_nodes[node.value].value(); }
  const N &get(NodeId node) const { return m_nodes[node.value].value(); }

  /**
   * \brief Returns an edge value.
   * \code{.cpp}
   * E& get(EdgeId edge);
   * const E& get(EdgeId edge) const;
   * \endcode
   * \param edge Live edge identifier.
   * \return The edge value.
   * \attention 1. The identifier must refer to a live edge.
   */
  E &get(EdgeId edge) { return m_edges[edge.value]->value; }
  const E &get(EdgeId edge) const { return m_edges[edge.value]->value; }

  /**
   * \brief Returns the source of an edge.
   * \code{.cpp}
   * NodeId source(EdgeId edge) const;
   * \endcode
   * \param edge Live edge identifier.
   * \return Source node identifier.
   * \attention 1. The identifier must refer to a live edge.
   */
  NodeId source(EdgeId edge) const { return m_edges[edge.value]->source; }

  /**
   * \brief Returns the destination of an edge.
   * \code{.cpp}
   * NodeId destination(EdgeId edge) const;
   * \endcode
   * \param edge Live edge identifier.
   * \return Destination node identifier.
   * \attention 1. The identifier must refer to a live edge.
   */
  NodeId destination(EdgeId edge) const {
    return m_edges[edge.value]->destination;
  }

  /**
   * \brief Returns the number of live nodes.
   * \code{.cpp}
   * std::size_t node_count() const noexcept;
   * \endcode
   * \return Number of live nodes.
   */
  std::size_t node_count() const noexcept {
    return m_nodes.size() - m_node_free.size();
  }

  /**
   * \brief Returns the number of live edges.
   * \code{.cpp}
   * std::size_t edge_count() const noexcept;
   * \endcode
   * \return Number of live edges.
   */
  std::size_t edge_count() const noexcept {
    return m_edges.size() - m_edge_free.size();
  }

  /**
   * \brief Returns the exclusive upper bound of node identifiers.
   * \code{.cpp}
   * std::size_t node_capacity() const noexcept;
   * \endcode
   * \return Exclusive upper node identifier bound.
   */
  std::size_t node_capacity() const noexcept { return m_nodes.size(); }

  /**
   * \brief Returns the exclusive upper bound of edge identifiers.
   * \code{.cpp}
   * std::size_t edge_capacity() const noexcept;
   * \endcode
   * \return Exclusive upper edge identifier bound.
   */
  std::size_t edge_capacity() const noexcept { return m_edges.size(); }

private:
  static void ensure_id_space(std::size_t size) {
    if (size >= std::numeric_limits<std::uint32_t>::max()) {
      throw std::length_error("graph identifier space exhausted");
    }
  }

  Vector<std::optional<N>, A> m_nodes;
  Vector<NodeId, A> m_node_free;
  Vector<std::optional<Edge>, A> m_edges;
  Vector<EdgeId, A> m_edge_free;
};

} // namespace strobe::graph
