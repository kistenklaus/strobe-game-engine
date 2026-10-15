#include <strobe/core/graph.hpp>

#include <gtest/gtest.h>

#include <cstddef>

namespace graph = strobe::graph;

TEST(TopologicalSort, OrdersEveryEdgeFromSourceToDestination) {
  graph::AdjGraph<int> builder;
  const auto first = builder.add_node(0);
  const auto second = builder.add_node(1);
  const auto third = builder.add_node(2);
  const auto fourth = builder.add_node(3);
  builder.add_edge(first, third);
  builder.add_edge(second, third);
  builder.add_edge(third, fourth);
  const graph::ConstGraph<int> value(std::move(builder));

  const auto result = graph::topological_sort(value);
  ASSERT_TRUE(result.acyclic);
  ASSERT_EQ(result.order.size(), value.node_count());
  strobe::Vector<std::size_t> position(value.node_count());
  for (std::size_t index = 0; index < result.order.size(); ++index) {
    position[result.order[index].value] = index;
  }
  for (std::uint32_t edge = 0; edge < value.edge_count(); ++edge) {
    EXPECT_LT(position[value.source(graph::EdgeId{edge}).value],
              position[value.destination(graph::EdgeId{edge}).value]);
  }
}

TEST(TopologicalSort, ReportsCyclesAndKeepsTheSortablePrefix) {
  graph::AdjGraph<int> builder;
  const auto root = builder.add_node(0);
  const auto first = builder.add_node(1);
  const auto second = builder.add_node(2);
  builder.add_edge(first, second);
  builder.add_edge(second, first);
  const graph::ConstGraph<int> value(std::move(builder));

  const auto result = graph::topological_sort(value);
  EXPECT_FALSE(result.acyclic);
  ASSERT_EQ(result.order.size(), 1);
  EXPECT_EQ(result.order[0], root);
}

TEST(StronglyConnectedComponents, PartitionsCyclesAndSingletons) {
  graph::AdjGraph<int> builder;
  const auto first = builder.add_node(0);
  const auto second = builder.add_node(1);
  const auto third = builder.add_node(2);
  const auto fourth = builder.add_node(3);
  const auto isolated = builder.add_node(4);
  builder.add_edge(first, second);
  builder.add_edge(second, first);
  builder.add_edge(second, third);
  builder.add_edge(third, fourth);
  builder.add_edge(fourth, third);
  const graph::ConstGraph<int> value(std::move(builder));

  const auto result = graph::strongly_connected_components(value);
  ASSERT_EQ(result.component_count(), 3);
  EXPECT_EQ(result.component_of(first), result.component_of(second));
  EXPECT_EQ(result.component_of(third), result.component_of(fourth));
  EXPECT_NE(result.component_of(first), result.component_of(third));
  EXPECT_NE(result.component_of(first), result.component_of(isolated));
  EXPECT_NE(result.component_of(third), result.component_of(isolated));
  EXPECT_EQ(result.component(result.component_of(first)).size(), 2);
  EXPECT_EQ(result.component(result.component_of(third)).size(), 2);
  EXPECT_EQ(result.component(result.component_of(isolated)).size(), 1);
}

TEST(StronglyConnectedComponents, HandlesSelfLoopsAndEmptyGraphs) {
  graph::AdjGraph<int> self_builder;
  const auto node = self_builder.add_node(0);
  self_builder.add_edge(node, node);
  const graph::ConstGraph<int> self(std::move(self_builder));
  const auto self_result = graph::strongly_connected_components(self);
  EXPECT_EQ(self_result.component_count(), 1);
  EXPECT_EQ(self_result.component(0).size(), 1);

  graph::AdjGraph<int> empty_builder;
  const graph::ConstGraph<int> empty(std::move(empty_builder));
  const auto empty_result = graph::strongly_connected_components(empty);
  EXPECT_EQ(empty_result.component_count(), 0);
  EXPECT_TRUE(graph::topological_sort(empty).acyclic);
}
