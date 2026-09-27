#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/file.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(strobe_fs_exists, file) {

  auto file = strobe::open("file-exists-test", strobe::fs::FileAccess::create |
                                          strobe::fs::FileAccess::write);

  ASSERT_TRUE(strobe::fs::exists("file-exists-test"));
  strobe::Path<> owning_path(strobe::PathView("file-exists-test"));
  ASSERT_TRUE(strobe::fs::exists(owning_path));
  ASSERT_FALSE(strobe::fs::exists("ajkhsdkajshd123"));

  strobe::fs::rm("file-exists-test");
}


// Basic allocation and deallocation
TEST(strobe_fs_exists, dir) {

  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_FALSE(strobe::fs::exists("ajkhsdkajshd123"));

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}
