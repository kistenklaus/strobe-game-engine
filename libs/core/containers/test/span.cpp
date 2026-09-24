#include "strobe/core/containers/span.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

TEST(Span, pointer_constructor_views_one_element) {
  int value = 42;
  strobe::span one(&value);
  strobe::span<int> empty(static_cast<int *>(nullptr));

  ASSERT_EQ(one.size(), 1);
  EXPECT_EQ(one.front(), 42);
  one[0] = 7;
  EXPECT_EQ(value, 7);

  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.data(), nullptr);
}

TEST(Span, constructs_from_arrays_and_contiguous_ranges) {
  int raw[]{1, 2, 3};
  std::array array{4, 5, 6};
  std::vector vector{7, 8, 9};

  strobe::span raw_view(raw);
  strobe::span array_view(array);
  strobe::span vector_view(vector);

  static_assert(decltype(raw_view)::extent == 3);
  static_assert(decltype(array_view)::extent == 3);
  static_assert(decltype(vector_view)::extent == strobe::dynamic_extent);

  EXPECT_TRUE(std::ranges::equal(raw_view, std::array{1, 2, 3}));
  EXPECT_TRUE(std::ranges::equal(array_view, array));
  EXPECT_TRUE(std::ranges::equal(vector_view, vector));

  vector_view[1] = 10;
  EXPECT_EQ(vector[1], 10);
}

TEST(Span, converts_extent_and_constness) {
  int values[]{2, 4, 6};
  strobe::span<int, 3> fixed(values);
  strobe::span<int> dynamic = fixed;
  strobe::span<const int> read_only = dynamic;
  strobe::span<const int, 3> fixed_read_only(dynamic);

  EXPECT_EQ(fixed_read_only.size(), 3);
  EXPECT_EQ(fixed_read_only.data(), values);
  EXPECT_EQ(read_only[2], 6);

  static_assert(
      std::is_constructible_v<strobe::span<const int>, strobe::span<int, 3>>);
  static_assert(
      !std::is_constructible_v<strobe::span<int>, strobe::span<const int, 3>>);
  static_assert(
      !std::is_constructible_v<strobe::span<int, 2>, strobe::span<int, 3>>);
}

TEST(Span, subviews_select_expected_elements) {
  int values[]{1, 2, 3, 4, 5};
  strobe::span view(values);

  EXPECT_TRUE(std::ranges::equal(view.first<2>(), std::array{1, 2}));
  EXPECT_TRUE(std::ranges::equal(view.last<2>(), std::array{4, 5}));
  EXPECT_TRUE(std::ranges::equal(view.subspan<1, 3>(), std::array{2, 3, 4}));
  EXPECT_TRUE(std::ranges::equal(view.subspan<2>(), std::array{3, 4, 5}));

  EXPECT_TRUE(std::ranges::equal(view.first(3), std::array{1, 2, 3}));
  EXPECT_TRUE(std::ranges::equal(view.last(2), std::array{4, 5}));
  EXPECT_TRUE(std::ranges::equal(view.subspan(1, 2), std::array{2, 3}));
  EXPECT_TRUE(std::ranges::equal(view.subspan(3), std::array{4, 5}));

  static_assert(decltype(view.first<2>())::extent == 2);
  static_assert(decltype(view.subspan<2>())::extent == 3);
}

TEST(Span, iterators_and_element_access) {
  int values[]{3, 5, 7};
  strobe::span view(values);

  EXPECT_EQ(view.front(), 3);
  EXPECT_EQ(view.back(), 7);
  EXPECT_EQ(view.at(1), 5);
  EXPECT_TRUE(std::ranges::equal(view, std::array{3, 5, 7}));
  EXPECT_TRUE(std::ranges::equal(
      std::ranges::subrange(view.rbegin(), view.rend()), std::array{7, 5, 3}));

  EXPECT_THROW(view.at(3), std::out_of_range);
}

TEST(Span, byte_views_cover_the_same_storage) {
  int values[]{1, 2};
  strobe::span view(values);

  auto bytes = strobe::as_bytes(view);
  auto writable = strobe::as_writable_bytes(view);

  EXPECT_EQ(bytes.size(), sizeof(values));
  EXPECT_EQ(writable.size(), sizeof(values));
  EXPECT_EQ(bytes.data(), static_cast<const void *>(values));
  EXPECT_EQ(writable.data(), static_cast<void *>(values));

  static_assert(decltype(bytes)::extent == sizeof(values));
  static_assert(std::is_same_v<decltype(bytes)::element_type, const std::byte>);
}
