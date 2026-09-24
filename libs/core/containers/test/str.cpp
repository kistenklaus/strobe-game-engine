#include "strobe/core/containers/str.hpp"

#include <gtest/gtest.h>

#include <array>
#include <ranges>
#include <string_view>
#include <type_traits>

TEST(Str, literal_construction_and_deduction) {
  strobe::Str text("hello");

  static_assert(decltype(text)::size() == 5);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');
  EXPECT_EQ(text.front(), 'h');
  EXPECT_EQ(text.back(), 'o');
}

TEST(Str, fixed_length_and_default_construction) {
  strobe::Str<3> text;

  EXPECT_EQ(text.size(), 3);
  EXPECT_FALSE(text.empty());
  EXPECT_EQ(text.view(), std::string_view("\0\0\0", 3));
  EXPECT_EQ(text.c_str()[3], '\0');

  strobe::Str<0> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.view(), "");
  EXPECT_EQ(empty.c_str()[0], '\0');
}

TEST(Str, constructs_from_a_view_with_embedded_null) {
  constexpr std::string_view input("a\0b", 3);
  strobe::Str<3> text(input);

  EXPECT_EQ(text.size(), 3);
  EXPECT_EQ(text.view(), input);
  EXPECT_EQ(text.c_str()[3], '\0');
}

TEST(Str, copies_have_independent_storage) {
  strobe::Str original("hello");
  auto copy = original;

  EXPECT_EQ(copy, original);
  EXPECT_NE(copy.data(), original.data());
}

TEST(Str, iteration_and_search) {
  strobe::Str text("one two");

  EXPECT_TRUE(std::ranges::equal(text, std::string_view("one two")));
  EXPECT_TRUE(std::ranges::equal(
      std::ranges::subrange(text.rbegin(), text.rend()),
      std::string_view("owt eno")));

  EXPECT_EQ(text.find("two"), 4);
  EXPECT_EQ(text.find('z'), decltype(text)::npos);
  EXPECT_TRUE(text.starts_with("one"));
  EXPECT_TRUE(text.ends_with("two"));
  EXPECT_TRUE(text.contains("e t"));
  EXPECT_EQ(text.substr(4), "two");
}

TEST(Str, comparison_uses_characters_and_length) {
  strobe::Str short_text("abc");
  strobe::Str same("abc");
  strobe::Str longer("abcd");
  strobe::Str later("abd");

  EXPECT_EQ(short_text, same);
  EXPECT_NE(short_text, longer);
  EXPECT_LT(short_text, longer);
  EXPECT_LT(short_text, later);
}

TEST(Str, access_is_read_only) {
  using Text = strobe::Str<3>;

  static_assert(std::is_same_v<decltype(std::declval<Text &>().data()),
                               const char *>);
  static_assert(std::is_same_v<decltype(std::declval<Text &>()[0]),
                               const char &>);
}

TEST(Str, supports_constant_evaluation) {
  constexpr strobe::Str text("hello");

  static_assert(text.size() == 5);
  static_assert(text.view() == "hello");
  static_assert(text.substr(1, 3) == "ell");
  static_assert(text.c_str()[5] == '\0');
}
