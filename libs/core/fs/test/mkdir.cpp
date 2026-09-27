#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/file.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

// Basic allocation and deallocation
TEST(mkdir, basic) {
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive | strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir", strobe::fs::MkdirFlags::parents);

  ASSERT_TRUE(strobe::fs::exists("testdir"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}


TEST(mkdir, parents) {
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive | strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir/xyz/foo", strobe::fs::MkdirFlags::parents);

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir/xyz/foo"));

  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz").isDirectory());
  ASSERT_TRUE(strobe::fs::stat("testdir/xyz/foo").isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}

TEST(mkdir, parents_is_idempotent_for_existing_directories) {
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive |
                                strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir/nested", strobe::fs::MkdirFlags::parents);
  ASSERT_NO_THROW(
      strobe::fs::mkdir("testdir/nested", strobe::fs::MkdirFlags::parents));

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}

TEST(mkdir, parents_normalizes_dot_components) {
  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive |
                                strobe::fs::RmFlags::force);

  strobe::fs::mkdir("testdir/./nested/../final/",
                    strobe::fs::MkdirFlags::parents);

  ASSERT_TRUE(strobe::fs::stat("testdir/final").isDirectory());
  ASSERT_FALSE(strobe::fs::exists("testdir/nested"));

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}

TEST(mkdir, parents_rejects_file_in_parent_chain) {
  strobe::fs::rm("mkdir-blocking-file", strobe::fs::RmFlags::force);
  {
    auto file = strobe::open("mkdir-blocking-file",
                             strobe::fs::FileAccess::create |
                                 strobe::fs::FileAccess::write);
  }

  ASSERT_THROW(strobe::fs::mkdir("mkdir-blocking-file/child",
                                 strobe::fs::MkdirFlags::parents),
               std::runtime_error);

  strobe::fs::rm("mkdir-blocking-file", strobe::fs::RmFlags::force);
}

TEST(mkdir, parents_rejects_empty_path) {
  ASSERT_THROW(strobe::fs::mkdir("", strobe::fs::MkdirFlags::parents),
               std::invalid_argument);
}
