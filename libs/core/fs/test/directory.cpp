#include "strobe/core/fs/directory.hpp"

#include <gtest/gtest.h>
#include <ranges>
#include <system_error>

static_assert(std::ranges::input_range<strobe::fs::Directory>);

TEST(Directory, IsAnInputRange) {
  auto directory = strobe::fs::ls(".");

  std::size_t count = 0;
  for (const auto entry : directory) {
    ASSERT_NE(entry.name.c_str(), nullptr);
    ASSERT_GT(entry.name.size(), 0u);
    EXPECT_STRNE(entry.name.c_str(), ".");
    EXPECT_STRNE(entry.name.c_str(), "..");
    ++count;
  }

  EXPECT_GT(count, 0u);
}

TEST(Directory, MissingDirectoryFailsToOpen) {
  EXPECT_THROW(strobe::fs::ls("directory-that-does-not-exist"),
               std::system_error);
}
