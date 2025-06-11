#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(strobe_fs_exists, basic) {

  strobe::File file("file-exists-test", strobe::FileAccessBits::Create |
                                          strobe::FileAccessBits::Write);

  ASSERT_TRUE(strobe::fs::exists("file-exists-test"));
  ASSERT_FALSE(strobe::fs::exists("ajkhsdkajshd123"));

  strobe::fs::rm("file-exists-test");
}
