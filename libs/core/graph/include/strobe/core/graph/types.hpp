#pragma once

#include <cstdint>

namespace strobe::graph {

/**
 * \brief Identifies a node in a graph.
 * \code{.cpp}
 * struct NodeId;
 * \endcode
 */
struct NodeId {
  std::uint32_t value;

  /**
   * \brief Compares two node identifiers.
   * \code{.cpp}
   * friend bool operator==(NodeId, NodeId) = default;
   * \endcode
   *
   * \return Whether the identifiers are equal.
   */
  friend bool operator==(NodeId, NodeId) = default;
};

/**
 * \brief Identifies an edge in a graph.
 * \code{.cpp}
 * struct EdgeId;
 * \endcode
 */
struct EdgeId {
  std::uint32_t value;

  /**
   * \brief Compares two edge identifiers.
   * \code{.cpp}
   * friend bool operator==(EdgeId, EdgeId) = default;
   * \endcode
   *
   * \return Whether the identifiers are equal.
   */
  friend bool operator==(EdgeId, EdgeId) = default;
};

/**
 * \brief Empty edge payload for graphs that only store connectivity.
 * \code{.cpp}
 * struct EmptyEdge;
 * \endcode
 */
struct EmptyEdge {};

} // namespace strobe::graph
