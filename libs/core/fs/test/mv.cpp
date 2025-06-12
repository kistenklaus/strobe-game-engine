#include "strobe/core/fs/mv.hpp"
#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(mv, move_file) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testfile-foo", strobe::fs::RmFlagBits::Force);

  {
    strobe::fs::File file("testfile", strobe::fs::FileAccessBits::Create |
                                      strobe::fs::FileAccessBits::Write);
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::mv("testfile", "testfile-foo");

  ASSERT_FALSE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::exists("testfile-foo"));
  ASSERT_TRUE(strobe::fs::stat("testfile-foo").isFile());

  strobe::fs::rm("testfile-foo");
}

TEST(mv, move_directory) {
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive |
                                strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-foo", strobe::fs::RmFlagBits::Recursive |
                                strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlagBits::Parents);

  strobe::fs::File file("testdir/testfile", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz/foo").isDirectory());

  strobe::fs::mv("testdir", "testdir-foo");

  ASSERT_FALSE(strobe::fs::exists("testdir"));
  ASSERT_FALSE(strobe::fs::exists("testdir/xyz"));
  ASSERT_FALSE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::exists("testdir-foo"));
  ASSERT_TRUE(strobe::fs::exists("testdir-foo/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir-foo/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir-foo").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir-foo/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir-foo/xyz/foo").isDirectory());

  strobe::fs::rm("testdir-foo", strobe::fs::RmFlagBits::Recursive);

  ASSERT_FALSE(strobe::fs::exists("testdir-foo"));
}
