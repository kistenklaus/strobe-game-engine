#include "strobe/core/containers/string.hpp"

#include <gtest/gtest.h>

#include <ranges>
#include <string_view>
#include <utility>

TEST(String, construction_and_null_termination) {
  strobe::String<> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.c_str()[0], '\0');

  strobe::String<> text("hello");
  EXPECT_EQ(text.size(), 5);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');

  strobe::String<> filled(5, 'x');
  EXPECT_EQ(filled.view(), "xxxxx");
  EXPECT_EQ(filled.c_str()[filled.size()], '\0');
}

TEST(String, append_resize_and_clear) {
  strobe::String<> text;

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

TEST(String, reserve_preserves_contents) {
  strobe::String<> text("hello");
  text.reserve(text.capacity() + 32);

  EXPECT_GE(text.capacity(), 37);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');
}

TEST(String, insert_erase_and_replace) {
  strobe::String<> text("ac");

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

TEST(String, self_referential_edits) {
  strobe::String<> text("abcd");

  text.append(text.view().substr(1, 2));
  EXPECT_EQ(text.view(), "abcdbc");

  text.assign("abcd");
  text.insert(1, text.view().substr(2, 2));
  EXPECT_EQ(text.view(), "acdbcd");

  text.assign("abcd");
  text.replace(1, 2, text.view().substr(0, 3));
  EXPECT_EQ(text.view(), "aabcd");

  text.assign("repeat");
  text.append(text.view());
  EXPECT_EQ(text.view(), "repeatrepeat");
  EXPECT_EQ(text.c_str()[text.size()], '\0');
}

TEST(String, views_iterators_and_search) {
  strobe::String<> text("one two");

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

TEST(String, copies_and_moves_preserve_values) {
  strobe::String<> original("hello");
  auto copy = original;

  copy[0] = 'H';
  EXPECT_EQ(original.view(), "hello");
  EXPECT_EQ(copy.view(), "Hello");

  strobe::String<> assigned;
  assigned = original;
  EXPECT_EQ(assigned.view(), "hello");

  auto moved = std::move(copy);
  EXPECT_EQ(moved.view(), "Hello");
  EXPECT_TRUE(copy.empty());
  EXPECT_EQ(copy.c_str()[0], '\0');

  strobe::String<> move_assigned;
  move_assigned = std::move(assigned);
  EXPECT_EQ(move_assigned.view(), "hello");
  EXPECT_TRUE(assigned.empty());
  EXPECT_EQ(assigned.c_str()[0], '\0');
}

TEST(String, embedded_null_is_part_of_the_string) {
  constexpr std::string_view input("a\0b", 3);
  strobe::String<> text(input);

  EXPECT_EQ(text.size(), 3);
  EXPECT_EQ(text.view(), input);
  EXPECT_EQ(text.c_str()[3], '\0');
}
