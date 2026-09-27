#include "strobe/core/fs/stat.hpp"
#include "strobe/core/fs/file.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>
#include <system_error>

// Basic allocation and deallocation
TEST(stat, basic_file) {
  strobe::fs::rm("testfile", strobe::fs::RmFlags::force);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    auto file = strobe::open("testfile", strobe::fs::FileAccess::create |
                                      strobe::fs::FileAccess::write);

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
  strobe::fs::rm("testdir", strobe::fs::RmFlags::force |
                                strobe::fs::RmFlags::recursive);

  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  auto stat = strobe::fs::stat("testdir");
  ASSERT_TRUE(stat.isDirectory());

  strobe::fs::rm("testdir", strobe::fs::RmFlags::recursive);
}

TEST(stat, missing_path_reports_an_error) {
  ASSERT_THROW(strobe::fs::stat("missing-stat-path"), std::system_error);
}

TEST(stat, follow_symlink_flag_is_accepted) {
  strobe::fs::rm("stat-follow-file", strobe::fs::RmFlags::force);
  auto file = strobe::open("stat-follow-file",
                           strobe::fs::FileAccess::create |
                               strobe::fs::FileAccess::write);

  const auto result = strobe::fs::stat(
      "stat-follow-file", strobe::fs::StatFlags::follow_symlink);
  ASSERT_TRUE(result.isFile());

  strobe::fs::rm("stat-follow-file", strobe::fs::RmFlags::force);
}
