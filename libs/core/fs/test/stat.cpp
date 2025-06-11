#include "strobe/core/fs/stat.hpp"
#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/FileAccessFlags.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(stat, basic_file) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::File file("testfile", strobe::FileAccessBits::Create |
                                      strobe::FileAccessBits::Write);

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  auto stat = strobe::fs::stat("testfile");
  ASSERT_TRUE(stat.isFile());
  ASSERT_EQ(stat.size(), bytes.size());

  strobe::fs::rm("testfile");
}

TEST(stat, basic_directory) {
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Force |
                                strobe::fs::RmFlagBits::Recursive);
  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  auto stat = strobe::fs::stat("testdir");
  ASSERT_TRUE(stat.isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
}
