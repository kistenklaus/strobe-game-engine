#include "strobe/core/fs/Path.hpp"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/rm.hpp"
#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>

#include <strobe/core/fs/File.hpp>

// Basic allocation and deallocation
TEST(File, Create) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                          strobe::fs::FileAccessBits::Write);
  ASSERT_TRUE(file);
  ASSERT_TRUE(file.isOpen());

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));
  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Write) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                          strobe::fs::FileAccessBits::Write);
  ASSERT_TRUE(file);
  ASSERT_TRUE(file.isOpen());

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));

  std::size_t written = 0;
  while (written != bytes.size()) {
    std::size_t w = file.write(bytes.subspan(written));
    written += w;
  }

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));
  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Read) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  {
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));

  {
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);

    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Seek) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  char text[] = "foobar";
  std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                             std::strlen(text));
  strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                          strobe::fs::FileAccessBits::Write);
  ASSERT_TRUE(file);
  ASSERT_TRUE(file.isOpen());

  std::size_t written = 0;
  while (written != bytes.size()) {
    std::size_t w = file.write(bytes.subspan(written));
    written += w;
  }

  file.seek(0);

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));

  {
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  file.seek(-static_cast<long>(bytes.size()), strobe::fs::FileSeekFlags::Cur);
  {
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  file.seek(-static_cast<long>(bytes.size()), strobe::fs::FileSeekFlags::End);
  {
    std::size_t bytesRead = 0;
    std::vector<std::byte> out(bytes.size());
    while (bytesRead != bytes.size()) {
      std::size_t r = file.read(std::span(out.begin() + bytesRead, out.end()));
      bytesRead += r;
    }
    ASSERT_TRUE(std::ranges::equal(out, bytes));
  }

  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Append) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  {
    char text[] = "foobar";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {
    char text[] = "foobar";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Append |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Read);
  ASSERT_TRUE(file);

  file.seek(0, strobe::fs::FileSeekFlags::Set);

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));

  {

    char text[] = "foobarfoobar";
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

  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Trunc) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  {
    char text[] = "foobar";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {
    char text[] = "foobar";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Trunc |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Read);
  ASSERT_TRUE(file);

  file.seek(0, strobe::fs::FileSeekFlags::Set);

  // Cleanup
  ASSERT_TRUE(strobe::fs::exists("file-open-test"));

  {

    char text[] = "foobar";
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

  strobe::fs::rm("file-open-test");
}

// Basic allocation and deallocation
TEST(File, Exclusive) {
  strobe::fs::rm("file-open-test", strobe::fs::RmFlagBits::Force);

  {
    char text[] = "foobar";
    std::span<std::byte> bytes(reinterpret_cast<std::byte *>(text),
                               std::strlen(text));
    strobe::fs::File file("file-open-test", strobe::fs::FileAccessBits::Create |
                                            strobe::fs::FileAccessBits::Write);
    ASSERT_TRUE(file);
    ASSERT_TRUE(file.isOpen());

    std::size_t written = 0;
    while (written != bytes.size()) {
      std::size_t w = file.write(bytes.subspan(written));
      written += w;
    }
  }

  {
    EXPECT_ANY_THROW(strobe::fs::File file("file-open-test",
                                       strobe::fs::FileAccessBits::Create |
                                           strobe::fs::FileAccessBits::Exclusive |
                                           strobe::fs::FileAccessBits::Write););
  }

  strobe::fs::rm("file-open-test");
}
