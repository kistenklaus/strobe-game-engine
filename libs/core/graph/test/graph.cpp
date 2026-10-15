#include <strobe/core/graph.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace graph = strobe::graph;

TEST(AdjGraph, AddsRemovesAndReusesIdentifiers) {
  graph::AdjGraph<int, int> value;
  const auto first = value.add_node(1);
  const auto second = value.add_node(2);
  const auto edge = value.add_edge(first, second, 12);

  EXPECT_EQ(value.node_count(), 2);
  EXPECT_EQ(value.edge_count(), 1);
  EXPECT_EQ(value.source(edge), first);
  EXPECT_EQ(value.destination(edge), second);
  EXPECT_EQ(value.get(edge), 12);

  EXPECT_TRUE(value.remove_node(first));
  EXPECT_FALSE(value.contains(edge));
  EXPECT_EQ(value.edge_count(), 0);
  const auto replacement = value.add_node(3);
  EXPECT_EQ(replacement, first);
  EXPECT_EQ(value.get(replacement), 3);
}

TEST(AdjGraph, RejectsRemovedEndpoints) {
  graph::AdjGraph<int> value;
  const auto first = value.add_node(1);
  const auto second = value.add_node(2);
  value.remove_node(first);

  EXPECT_THROW(value.add_edge(first, second), std::invalid_argument);
}

TEST(ConstGraph, CompactsSparseIdentifiersAndBuildsAdjacency) {
  graph::AdjGraph<int, int> builder;
  const auto removed = builder.add_node(0);
  const auto first = builder.add_node(1);
  const auto second = builder.add_node(2);
  const auto third = builder.add_node(3);
  builder.remove_node(removed);
  builder.add_edge(first, second, 12);
  builder.add_edge(first, third, 13);
  builder.add_edge(second, third, 23);

  const graph::ConstGraph<int, int> value(builder);
  ASSERT_EQ(value.node_count(), 3);
  ASSERT_EQ(value.edge_count(), 3);
  EXPECT_EQ(value.get(graph::NodeId{0}), 1);
  EXPECT_EQ(value.get(graph::NodeId{1}), 2);
  EXPECT_EQ(value.get(graph::NodeId{2}), 3);
  ASSERT_EQ(value.outgoing(graph::NodeId{0}).size(), 2);
  EXPECT_EQ(value.destination(value.outgoing(graph::NodeId{0})[0]),
            graph::NodeId{1});
  EXPECT_EQ(value.destination(value.outgoing(graph::NodeId{0})[1]),
            graph::NodeId{2});
  ASSERT_EQ(value.incoming(graph::NodeId{2}).size(), 2);
  EXPECT_EQ(value.get(value.incoming(graph::NodeId{2})[0]), 13);
  EXPECT_EQ(value.get(value.incoming(graph::NodeId{2})[1]), 23);
}

TEST(ConstGraph, MovesValuesFromBuilder) {
  graph::AdjGraph<std::unique_ptr<int>, std::unique_ptr<int>> builder;
  const auto first = builder.add_node(std::make_unique<int>(1));
  const auto second = builder.add_node(std::make_unique<int>(2));
  builder.add_edge(first, second, std::make_unique<int>(3));

  const graph::ConstGraph<std::unique_ptr<int>, std::unique_ptr<int>> value(
      std::move(builder));
  EXPECT_EQ(*value.get(graph::NodeId{0}), 1);
  EXPECT_EQ(*value.get(graph::NodeId{1}), 2);
  EXPECT_EQ(*value.get(graph::EdgeId{0}), 3);
}

TEST(ConstGraph, ReturnsEmptyAdjacencyForIsolatedNodes) {
  graph::AdjGraph<int> builder;
  builder.add_node(1);

  const graph::ConstGraph<int> value(builder);
  EXPECT_TRUE(value.incoming(graph::NodeId{0}).empty());
  EXPECT_TRUE(value.outgoing(graph::NodeId{0}).empty());
}
