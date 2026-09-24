#include "strobe/core/containers/inplace_string.hpp"

#include <gtest/gtest.h>

#include <array>
#include <ranges>
#include <string_view>
#include <utility>

TEST(InplaceString, construction_and_null_termination) {
  strobe::InplaceString<8> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.capacity(), 8);
  EXPECT_EQ(empty.c_str()[0], '\0');

  strobe::InplaceString<8> text("hello");
  EXPECT_EQ(text.size(), 5);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');

  strobe::InplaceString<8> filled(8, 'x');
  EXPECT_EQ(filled.size(), filled.capacity());
  EXPECT_EQ(filled.view(), "xxxxxxxx");
  EXPECT_EQ(filled.c_str()[8], '\0');
}

TEST(InplaceString, append_and_resize) {
  strobe::InplaceString<8> text;
  text.push_back('a');
  text.append("bc");
  text.append(2, 'd');

  EXPECT_EQ(text.view(), "abcdd");
  EXPECT_EQ(text.front(), 'a');
  EXPECT_EQ(text.back(), 'd');

  text.pop_back();
  text.resize(6, 'x');
  EXPECT_EQ(text.view(), "abcdxx");

  text.resize(2);
  EXPECT_EQ(text.view(), "ab");
  EXPECT_EQ(text.c_str()[text.size()], '\0');

  text.clear();
  EXPECT_TRUE(text.empty());
  EXPECT_EQ(text.c_str()[0], '\0');
}

TEST(InplaceString, insert_erase_and_replace) {
  strobe::InplaceString<16> text("ac");

  text.insert(1, "b");
  EXPECT_EQ(text.view(), "abc");

  text.insert(0, "<");
  text.append(">");
  EXPECT_EQ(text.view(), "<abc>");

  text.erase(1, 2);
  EXPECT_EQ(text.view(), "<c>");

  text.replace(1, 1, "hello");
  EXPECT_EQ(text.view(), "<hello>");
  EXPECT_EQ(text.c_str()[text.size()], '\0');
}

TEST(InplaceString, self_referential_edits) {
  strobe::InplaceString<16> text("abcd");

  text.append(text.view().substr(1, 2));
  EXPECT_EQ(text.view(), "abcdbc");

  text.assign("abcd");
  text.insert(1, text.view().substr(2, 2));
  EXPECT_EQ(text.view(), "acdbcd");

  text.assign("abcd");
  text.replace(1, 2, text.view().substr(0, 3));
  EXPECT_EQ(text.view(), "aabcd");
}

TEST(InplaceString, views_iterators_and_search) {
  strobe::InplaceString<12> text("one two");

  EXPECT_TRUE(std::ranges::equal(text, std::string_view("one two")));
  EXPECT_TRUE(std::ranges::equal(
      std::ranges::subrange(text.rbegin(), text.rend()),
      std::string_view("owt eno")));

  EXPECT_EQ(text.find("two"), 4);
  EXPECT_EQ(text.find('z'), decltype(text)::npos);
  EXPECT_TRUE(text.starts_with("one"));
  EXPECT_TRUE(text.ends_with("two"));
  EXPECT_TRUE(text.contains("e t"));

  text[0] = 'O';
  EXPECT_EQ(text.view(), "One two");
}

TEST(InplaceString, copies_and_moves_are_independent) {
  strobe::InplaceString<8> original("hello");
  auto copy = original;

  copy[0] = 'H';
  EXPECT_EQ(original.view(), "hello");
  EXPECT_EQ(copy.view(), "Hello");

  strobe::InplaceString<8> assigned;
  assigned = original;
  EXPECT_EQ(assigned.view(), "hello");

  auto moved = std::move(copy);
  EXPECT_EQ(moved.view(), "Hello");
  EXPECT_EQ(moved.c_str()[moved.size()], '\0');
}

TEST(InplaceString, zero_capacity_represents_empty_string) {
  strobe::InplaceString<0> text;

  EXPECT_TRUE(text.empty());
  EXPECT_EQ(text.capacity(), 0);
  EXPECT_EQ(text.view(), "");
  EXPECT_EQ(text.c_str()[0], '\0');
}
