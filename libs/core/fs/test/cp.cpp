#include "strobe/core/fs/cp.hpp"
#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/mkdir.hpp"
#include "strobe/core/fs/rm.hpp"
#include "strobe/core/fs/stat.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(cp, cp_empty_file) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testfile-foo", strobe::fs::RmFlagBits::Force);

  {
    strobe::fs::File file("testfile", strobe::fs::FileAccessBits::Create |
                                      strobe::fs::FileAccessBits::Write);
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::cp("testfile", "testfile-foo");

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  ASSERT_TRUE(strobe::fs::exists("testfile-foo"));
  ASSERT_TRUE(strobe::fs::stat("testfile-foo").isFile());

  strobe::fs::rm("testfile-foo");
  strobe::fs::rm("testfile");
}

// Basic allocation and deallocation
TEST(cp, cp_file_content) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testfile-foo", strobe::fs::RmFlagBits::Force);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::fs::File file("testfile", strobe::fs::FileAccessBits::Create |
                                      strobe::fs::FileAccessBits::Write);

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::cp("testfile", "testfile-foo");

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  ASSERT_TRUE(strobe::fs::exists("testfile-foo"));
  ASSERT_TRUE(strobe::fs::stat("testfile-foo").isFile());

  {
    strobe::fs::File file("testfile-foo", strobe::fs::FileAccessBits::Read);
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("testfile-foo");
  strobe::fs::rm("testfile");
}

// Basic allocation and deallocation
TEST(cp, cp_file_into_dir) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Force |
                                strobe::fs::RmFlagBits::Recursive);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::fs::File file("testfile", strobe::fs::FileAccessBits::Create |
                                      strobe::fs::FileAccessBits::Write);

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());

  strobe::fs::cp("testfile", "testdir");

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  ASSERT_TRUE(strobe::fs::exists("testdir/testfile"));
  ASSERT_TRUE(strobe::fs::stat("testdir/testfile").isFile());

  {
    strobe::fs::File file("testdir/testfile", strobe::fs::FileAccessBits::Read);
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
}

// Basic allocation and deallocation
TEST(cp, cp_file_into_dir_without_force) {
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Force |
                                strobe::fs::RmFlagBits::Recursive);

  {
    strobe::fs::File file("testfile", strobe::fs::FileAccessBits::Create |
                                      strobe::fs::FileAccessBits::Write);
  }

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  strobe::fs::mkdir("testdir");

  ASSERT_TRUE(strobe::fs::exists("testdir"));
  ASSERT_TRUE(strobe::fs::stat("testdir").isDirectory());

  {
    strobe::fs::File file("testdir/testfile", strobe::fs::FileAccessBits::Create |
                                              strobe::fs::FileAccessBits::Write);
  }

  strobe::fs::cp("testfile", "testdir"); // does not require force!

  ASSERT_TRUE(strobe::fs::exists("testfile"));
  ASSERT_TRUE(strobe::fs::stat("testfile").isFile());

  ASSERT_TRUE(strobe::fs::exists("testdir/testfile"));
  ASSERT_TRUE(strobe::fs::stat("testdir/testfile").isFile());

  strobe::fs::rm("testdir", strobe::fs::RmFlagBits::Recursive);
  strobe::fs::rm("testfile", strobe::fs::RmFlagBits::Force);
}

TEST(cp, cp_empty_dir) {
  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir-a");
  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());

  strobe::fs::cp("testdir-a", "testdir-b", strobe::fs::CpFlagBits::Recursive);

  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());
  ASSERT_TRUE(strobe::fs::exists("testdir-b"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b").isDirectory());

  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
}

TEST(cp, cp_dir) {
  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir-a/foo/bar", strobe::fs::MkdirFlagBits::Parents);
  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::fs::File file("testdir-a/abc", strobe::fs::FileAccessBits::Write |
                                           strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Exclusive |
                                           strobe::fs::FileAccessBits::Trunc);
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }
  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());

  strobe::fs::cp("testdir-a", "testdir-b", strobe::fs::CpFlagBits::Recursive);

  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());
  ASSERT_TRUE(strobe::fs::exists("testdir-b"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b").isDirectory());

  ASSERT_TRUE(strobe::fs::exists("testdir-b/abc"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/abc").isFile());
  {
    strobe::fs::File file("testdir-b/abc", strobe::fs::FileAccessBits::Read);

    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  ASSERT_TRUE(strobe::fs::exists("testdir-b/foo"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/foo").isDirectory());
  ASSERT_TRUE(strobe::fs::exists("testdir-b/foo/bar"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/foo/bar").isDirectory());

  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
}

TEST(cp, cp_dir_into_dir) {
  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir-a/foo/bar", strobe::fs::MkdirFlagBits::Parents);
  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::fs::File file("testdir-a/abc", strobe::fs::FileAccessBits::Write |
                                           strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Exclusive |
                                           strobe::fs::FileAccessBits::Trunc);
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }
  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());

  strobe::fs::mkdir("testdir-b");
  ASSERT_TRUE(strobe::fs::exists("testdir-b"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b").isDirectory());

  strobe::fs::cp("testdir-a", "testdir-b", strobe::fs::CpFlagBits::Recursive);

  return;

  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-a").isDirectory());

  ASSERT_TRUE(strobe::fs::exists("testdir-b"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b").isDirectory());
  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/testdir-a").isDirectory());

  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/abc/"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/testdir-a/abc").isFile());

  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/foo/"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/testdir-a/foo").isDirectory());

  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/foo/bar"));
  ASSERT_TRUE(strobe::fs::stat("testdir-b/testdir-a/foo/bar").isDirectory());

  {
    strobe::fs::File file("testdir-b/testdir-a/abc", strobe::fs::FileAccessBits::Read);

    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
}

TEST(cp, cp_dir_existing_dir) {

  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive |
                                  strobe::fs::RmFlagBits::Force);

  strobe::fs::mkdir("testdir-a");
  strobe::fs::mkdir("testdir-a/foo");
  strobe::fs::mkdir("testdir-a/foo/bar");

  strobe::fs::mkdir("testdir-b");
  strobe::fs::mkdir("testdir-b/foo");
  strobe::fs::mkdir("testdir-b/foo/bar");

  {

    strobe::fs::File file("testdir-a/abc", strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Write |
                                           strobe::fs::FileAccessBits::Trunc);

    char text[] = "testdir-a/abc";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {

    strobe::fs::File file("testdir-a/foo/xyz", strobe::fs::FileAccessBits::Create |
                                               strobe::fs::FileAccessBits::Write |
                                               strobe::fs::FileAccessBits::Trunc);

    char text[] = "testdir-a/foo/xyz";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {
    strobe::fs::File file("testdir-b/abc", strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Write |
                                           strobe::fs::FileAccessBits::Trunc);

    char text[] = "testdir-b/abc";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {
    strobe::fs::File file("testdir-b/baz", strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Write |
                                           strobe::fs::FileAccessBits::Trunc);

    char text[] = "testdir-b/baz";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  strobe::fs::cp("testdir-a", "testdir-b", strobe::fs::CpFlagBits::Recursive);

  ASSERT_TRUE(strobe::fs::exists("testdir-a"));
  ASSERT_TRUE(strobe::fs::exists("testdir-a/foo"));
  ASSERT_TRUE(strobe::fs::exists("testdir-a/foo/bar"));
  ASSERT_TRUE(strobe::fs::exists("testdir-a/abc"));
  ASSERT_TRUE(strobe::fs::exists("testdir-a/foo/xyz"));
  ASSERT_FALSE(strobe::fs::exists("testdir-b/testdir-a/baz"));

  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a"));
  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/foo"));
  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/foo/bar"));
  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/abc"));
  ASSERT_TRUE(strobe::fs::exists("testdir-b/testdir-a/foo/xyz"));
  ASSERT_TRUE(strobe::fs::exists("testdir-b/baz"));

  {

    strobe::fs::File file("testdir-b/testdir-a/abc", strobe::fs::FileAccessBits::Read);

    char text[] = "testdir-a/abc";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  {

    strobe::fs::File file("testdir-b/testdir-a/foo/xyz",
                      strobe::fs::FileAccessBits::Read);

    char text[] = "testdir-a/foo/xyz";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  {
    strobe::fs::File file("testdir-a/abc", strobe::fs::FileAccessBits::Read);

    char text[] = "testdir-a/abc";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  {
    strobe::fs::File file("testdir-b/baz", strobe::fs::FileAccessBits::Read);

    char text[] = "testdir-b/baz";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("testdir-a", strobe::fs::RmFlagBits::Recursive);
  strobe::fs::rm("testdir-b", strobe::fs::RmFlagBits::Recursive);
}
