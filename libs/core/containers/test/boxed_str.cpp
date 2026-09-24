#include "strobe/core/containers/boxed_str.hpp"

#include <gtest/gtest.h>

#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

TEST(BoxedStr, construction_and_null_termination) {
  strobe::BoxedStr<> empty(size_t{0});
  EXPECT_TRUE(empty.empty());
  EXPECT_NE(empty.c_str(), nullptr);
  EXPECT_EQ(empty.c_str()[0], '\0');

  strobe::BoxedStr<> text("hello");
  EXPECT_EQ(text.size(), 5);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');
  EXPECT_EQ(text.front(), 'h');
  EXPECT_EQ(text.back(), 'o');

  strobe::BoxedStr<> fixed(size_t{3});
  EXPECT_EQ(fixed.view(), std::string_view("\0\0\0", 3));
  EXPECT_EQ(fixed.c_str()[3], '\0');
}

TEST(BoxedStr, owns_embedded_nulls) {
  constexpr std::string_view input("a\0b", 3);
  strobe::BoxedStr<> text(input);

  EXPECT_EQ(text.size(), 3);
  EXPECT_EQ(text.view(), input);
  EXPECT_EQ(text.c_str()[3], '\0');
}

TEST(BoxedStr, copies_have_independent_storage) {
  strobe::BoxedStr<> original("hello");
  strobe::BoxedStr<> copy(original);

  EXPECT_EQ(copy, original);
  EXPECT_NE(copy.data(), original.data());

  strobe::BoxedStr<> assigned("old");
  assigned = original;
  EXPECT_EQ(assigned.view(), original.view());
  EXPECT_NE(assigned.data(), original.data());
}

TEST(BoxedStr, moves_preserve_value) {
  strobe::BoxedStr<> source("hello");
  const char *original_data = source.data();

  strobe::BoxedStr<> moved(std::move(source));
  EXPECT_EQ(moved.view(), "hello");
  EXPECT_EQ(moved.data(), original_data);
  EXPECT_TRUE(source.empty());

  strobe::BoxedStr<> target("old");
  target = std::move(moved);
  EXPECT_EQ(target.view(), "hello");
  EXPECT_EQ(target.data(), original_data);
  EXPECT_TRUE(moved.empty());
}

TEST(BoxedStr, iteration_and_search) {
  strobe::BoxedStr<> text("one two");

  EXPECT_TRUE(std::ranges::equal(text, std::string_view("one two")));
  EXPECT_TRUE(
      std::ranges::equal(std::ranges::subrange(text.rbegin(), text.rend()),
                         std::string_view("owt eno")));

  EXPECT_EQ(text.find("two"), 4);
  EXPECT_EQ(text.find('z'), decltype(text)::npos);
  EXPECT_TRUE(text.starts_with("one"));
  EXPECT_TRUE(text.ends_with("two"));
  EXPECT_TRUE(text.contains("e t"));
  EXPECT_EQ(text.substr(4), "two");
}

TEST(BoxedStr, comparisons_use_characters_and_length) {
  strobe::BoxedStr<> first("abc");
  strobe::BoxedStr<> same("abc");
  strobe::BoxedStr<> longer("abcd");
  strobe::BoxedStr<> later("abd");

  EXPECT_EQ(first, same);
  EXPECT_NE(first, longer);
  EXPECT_LT(first, longer);
  EXPECT_LT(first, later);
}

TEST(BoxedStr, exposes_read_only_characters) {
  using Text = strobe::BoxedStr<>;

  static_assert(
      std::is_same_v<decltype(std::declval<Text &>().data()), const char *>);
  static_assert(
      std::is_same_v<decltype(std::declval<Text &>()[0]), const char &>);
  static_assert(sizeof(Text) == sizeof(void *));
}
