#include "strobe/core/fs/file.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

// Basic allocation and deallocation
TEST(rm, basic) {
  strobe::fs::rm("testfile", strobe::fs::RmFlags::force);

  auto file = strobe::open("testfile", strobe::fs::FileAccess::create | strobe::fs::FileAccess::write);

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::rm("testfile");
}


TEST(rm, recursive) {
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive | strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlags::parents);

  auto file = strobe::open("testdir/testfile", strobe::fs::FileAccess::create | strobe::fs::FileAccess::write);

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz/foo").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);

  ASSERT_FALSE(strobe::fs::exists("testdir"));
}

TEST(rm, force_ignores_missing_paths) {
  strobe::fs::rm("missing-rm-path", strobe::fs::RmFlags::force);
  ASSERT_THROW(strobe::fs::rm("missing-rm-path"), std::invalid_argument);
}

TEST(rm, recursive_is_required_for_directories) {
  strobe::fs::rm("nonrecursive-rm-dir", strobe::fs::RmFlags::recursive |
                                             strobe::fs::RmFlags::force);
  strobe::fs::mkdir("nonrecursive-rm-dir");

  ASSERT_THROW(strobe::fs::rm("nonrecursive-rm-dir"), std::invalid_argument);
  ASSERT_TRUE(strobe::fs::exists("nonrecursive-rm-dir"));

  strobe::fs::rm("nonrecursive-rm-dir", strobe::fs::RmFlags::recursive);
}
