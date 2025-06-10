#include "strobe/core/fs/FileAccessFlags.hpp"
#include "strobe/core/fs/Path.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/rm.hpp"
#include <gtest/gtest.h>

#include <strobe/core/fs/File.hpp>

// Basic allocation and deallocation
TEST(File, Open) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  strobe::File file("file-open-test", strobe::FileAccessBits::Create | strobe::FileAccessBits::Write);
  ASSERT_TRUE(file);
  ASSERT_TRUE(file.isOpen());

  // Cleanup
  strobe::fs::exists("file-open-test");
  strobe::fs::rm("file-open-test");
}
