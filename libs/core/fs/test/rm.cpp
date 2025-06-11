#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/FileAccessFlags.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(rm, basic) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);

  strobe::File file("testfile", strobe::FileAccessBits::Create | strobe::FileAccessBits::Write);

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::rm("testfile");
}


TEST(rm, recursive) {
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive | strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlagBits::Parents);

  strobe::File file("testdir/testfile", strobe::FileAccessBits::Create | strobe::FileAccessBits::Write);

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz/foo").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);

  ASSERT_FALSE(strobe::fs::exists("testdir"));
}
