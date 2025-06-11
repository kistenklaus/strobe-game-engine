#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(mkdir, basic) {
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive | strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir", strobe::fs::MkdirFlagBits::Parents);

  ASSERT_TRUE(strobe::fs::exists("testdir"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
}


TEST(mkdir, parents) {
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive | strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlagBits::Parents);

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz/foo").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
}
