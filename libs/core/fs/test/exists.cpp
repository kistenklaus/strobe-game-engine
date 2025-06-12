#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(strobe_fs_exists, file) {

  strobe::fs::File file("file-exists-test", strobe::fs::FileAccessBits::Create |
                                          strobe::fs::FileAccessBits::Write);

  ASSERT_TRUE(strobe::fs::exists("file-exists-test"));
  ASSERT_FALSE(strobe::fs::exists("ajkhsdkajshd123"));

  strobe::fs::rm("file-exists-test");
}


// Basic allocation and deallocation
TEST(strobe_fs_exists, dir) {

  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_FALSE(strobe::fs::exists("ajkhsdkajshd123"));

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
}
