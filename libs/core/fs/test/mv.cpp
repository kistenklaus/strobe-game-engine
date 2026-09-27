#include "strobe/core/fs/mv.hpp"
#include "strobe/core/fs/file.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

// Basic allocation and deallocation
TEST(mv, move_file) {
  strobe::fs::rm("testfile", strobe::fs::RmFlags::force);
  strobe::fs::rm("testfile-foo", strobe::fs::RmFlags::force);

  {
    auto file = strobe::open("testfile", strobe::fs::FileAccess::create |
                                      strobe::fs::FileAccess::write);
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
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive |
                                strobe::fs::RmFlags::force);
  strobe::fs::rm("testdir-foo", strobe::fs::RmFlags::recursive |
                                strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlags::parents);

  auto file = strobe::open("testdir/testfile", strobe::fs::FileAccess::create |
                                            strobe::fs::FileAccess::write);

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

  strobe::fs::rm("testdir-foo", strobe::fs::RmFlags::recursive);

  ASSERT_FALSE(strobe::fs::exists("testdir-foo"));
}

TEST(mv, refuses_to_replace_without_force) {
  strobe::fs::rm("mv-source", strobe::fs::RmFlags::force);
  strobe::fs::rm("mv-destination", strobe::fs::RmFlags::force);
  auto source = strobe::open("mv-source", strobe::fs::FileAccess::create |
                                           strobe::fs::FileAccess::write);
  auto destination =
      strobe::open("mv-destination", strobe::fs::FileAccess::create |
                                           strobe::fs::FileAccess::write);

  ASSERT_THROW(strobe::fs::mv("mv-source", "mv-destination"),
               std::runtime_error);
  ASSERT_TRUE(strobe::fs::exists("mv-source"));
  ASSERT_TRUE(strobe::fs::exists("mv-destination"));

  strobe::fs::rm("mv-source", strobe::fs::RmFlags::force);
  strobe::fs::rm("mv-destination", strobe::fs::RmFlags::force);
}

TEST(mv, force_replaces_existing_file) {
  strobe::fs::rm("mv-source", strobe::fs::RmFlags::force);
  strobe::fs::rm("mv-destination", strobe::fs::RmFlags::force);
  auto source = strobe::open("mv-source", strobe::fs::FileAccess::create |
                                           strobe::fs::FileAccess::write);
  auto destination =
      strobe::open("mv-destination", strobe::fs::FileAccess::create |
                                           strobe::fs::FileAccess::write);

  ASSERT_NO_THROW(strobe::fs::mv("mv-source", "mv-destination",
                                 strobe::fs::MvFlags::force));
  ASSERT_FALSE(strobe::fs::exists("mv-source"));
  ASSERT_TRUE(strobe::fs::exists("mv-destination"));

  strobe::fs::rm("mv-destination", strobe::fs::RmFlags::force);
}
