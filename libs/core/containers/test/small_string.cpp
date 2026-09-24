#include "strobe/core/containers/small_string.hpp"

#include <gtest/gtest.h>

#include <ranges>
#include <string_view>
#include <utility>

TEST(SmallString, construction_and_null_termination) {
  strobe::SmallString<> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.c_str()[0], '\0');

  strobe::SmallString<> text("hello");
  EXPECT_EQ(text.size(), 5);
  EXPECT_EQ(text.view(), "hello");
  EXPECT_EQ(text.c_str()[text.size()], '\0');

  strobe::SmallString<4> filled(4, 'x');
  EXPECT_EQ(filled.view(), "xxxx");
  EXPECT_EQ(filled.c_str()[filled.size()], '\0');
}

TEST(SmallString, append_resize_and_clear) {
  strobe::SmallString<4> text;

  text.push_back('a');
  text.append("bc");
  text.append(2, 'd'); // Exceeds inline capacity.

  EXPECT_EQ(text.view(), "abcdd");
  EXPECT_EQ(text.front(), 'a');
  EXPECT_EQ(text.back(), 'd');
  EXPECT_EQ(text.c_str()[text.size()], '\0');

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

TEST(SmallString, insert_erase_and_replace) {
  strobe::SmallString<4> text("ac");

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

TEST(SmallString, self_referential_edits) {
  strobe::SmallString<4> text("abcd");

  text.append(text.view().substr(1, 2));
  EXPECT_EQ(text.view(), "abcdbc");

  text.assign("abcd");
  text.insert(1, text.view().substr(2, 2));
  EXPECT_EQ(text.view(), "acdbcd");

  text.assign("abcd");
  text.replace(1, 2, text.view().substr(0, 3));
  EXPECT_EQ(text.view(), "aabcd");
}

TEST(SmallString, views_iterators_and_search) {
  strobe::SmallString<4> text("one two");

  EXPECT_TRUE(std::ranges::equal(text, std::string_view("one two")));
  EXPECT_TRUE(
      std::ranges::equal(std::ranges::subrange(text.rbegin(), text.rend()),
                         std::string_view("owt eno")));

  EXPECT_EQ(text.find("two"), 4);
  EXPECT_EQ(text.find('z'), decltype(text)::npos);
  EXPECT_TRUE(text.starts_with("one"));
  EXPECT_TRUE(text.ends_with("two"));
  EXPECT_TRUE(text.contains("e t"));

  text[0] = 'O';
  EXPECT_EQ(text.view(), "One two");
}

TEST(SmallString, copies_and_moves_preserve_values) {
  strobe::SmallString<4> inline_text("abc");
  auto inline_copy = inline_text;
  inline_copy[0] = 'x';
  EXPECT_EQ(inline_text.view(), "abc");
  EXPECT_EQ(inline_copy.view(), "xbc");

  auto inline_moved = std::move(inline_copy);
  EXPECT_EQ(inline_moved.view(), "xbc");
  EXPECT_TRUE(inline_copy.empty());
  EXPECT_EQ(inline_copy.c_str()[0], '\0');

  strobe::SmallString<4> heap_text("a longer string");
  strobe::SmallString<4> heap_copy;
  heap_copy = heap_text;
  EXPECT_EQ(heap_copy.view(), heap_text.view());

  strobe::SmallString<4> heap_moved;
  heap_moved = std::move(heap_copy);
  EXPECT_EQ(heap_moved.view(), "a longer string");
  EXPECT_TRUE(heap_copy.empty());
  EXPECT_EQ(heap_copy.c_str()[0], '\0');
}

TEST(SmallString, embedded_null_is_part_of_the_string) {
  constexpr std::string_view input("a\0b", 3);
  strobe::SmallString<2> text(input);

  EXPECT_EQ(text.size(), 3);
  EXPECT_EQ(text.view(), input);
  EXPECT_EQ(text.c_str()[3], '\0');
}
