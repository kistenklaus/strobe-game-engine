#include "strobe/core/fs/path.hpp"
#include <array>
#include <gtest/gtest.h>

TEST(PathView, NullTerminatedLogicalRange) {
  const std::array<char, 4> storage{'a', 'b', 'c', '\0'};
  strobe::PathView view{std::span<const char>(storage)};

  EXPECT_EQ(view.size(), 3);
  EXPECT_EQ(view.view(), "abc");
  EXPECT_EQ(view.span().size(), 3);
  EXPECT_EQ(view.c_str()[view.size()], '\0');
}

TEST(PathView, NameAndExtensionUseTheFinalComponent) {
  strobe::PathView file("assets.v1/shader.spv");
  EXPECT_EQ(file.name(), "shader.spv");
  EXPECT_EQ(file.extension(), "spv");
  EXPECT_STREQ(file.c_str_name(), "shader.spv");

  strobe::PathView directory("assets.v1/shaders/");
  EXPECT_EQ(directory.name(), "shaders");
  EXPECT_TRUE(directory.extension().empty());
  EXPECT_STREQ(directory.c_str_name(), "shaders/");

  strobe::PathView hidden(".gitignore");
  EXPECT_EQ(hidden.name(), ".gitignore");
  EXPECT_TRUE(hidden.extension().empty());
}

// Basic allocation and deallocation
TEST(Path, Basic) {

  {
    strobe::Path<> path("foo");
    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo");
  }
}

TEST(Path, AccessorsAndEmptyNormalization) {
  strobe::Path<> path("assets.v1/shader.spv");

  EXPECT_EQ(path.size(), 20);
  EXPECT_EQ(path.view(), "assets.v1/shader.spv");
  EXPECT_EQ(path.name(), "shader.spv");
  EXPECT_EQ(path.extension(), "spv");
  EXPECT_STREQ(path.c_str_name(), "shader.spv");

  strobe::Path<> empty("");
  empty.normalize();
  EXPECT_STREQ(empty.c_str(), ".");
}

TEST(Path, Append) {
  {
    strobe::Path<> path("foo");

    path.append("abc");
    path.normalize();

    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo/abc");
  }
  {
    strobe::Path<> path("foo/");

    path.append("abc/");
    path.normalize();

    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo/abc/");
  }
}

TEST(Path, SlashAppend) {
  strobe::Path<> path("foo");
  path /= strobe::PathView("bar/");
  path.normalize();
  ASSERT_STREQ(path.c_str(), "foo/bar/");

  auto joined = path / strobe::PathView("baz");
  joined.normalize();
  ASSERT_STREQ(joined.c_str(), "foo/bar/baz");
  ASSERT_STREQ(path.c_str(), "foo/bar/");
}

TEST(Path, Parent) {
  {
    strobe::Path<> path("foo/abc");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "foo/");
  }

  {
    strobe::Path<> path("");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }

  {
    strobe::Path<> path("./");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }

  {
    strobe::Path<> path("../");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../../");
  }
}

TEST(Path, ClimbingParent) {
  {
    strobe::Path<> path("foo/");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "./");
  }
  {
    strobe::Path<> path("./");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }
}

TEST(Path, BasicNormalization) {

  {
    strobe::Path<> path("foo/./abc");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "foo/abc");
  }

  {
    strobe::Path<> path("foo/bar/../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "foo/abc/");
  }

  {
    strobe::Path<> path("foo/../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "abc/");
  }

  {
    strobe::Path<> path("xyz/foo/../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "abc/");
  }
}

TEST(Path, ClimbingNormalization) {
  {
    strobe::Path<> path("../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../abc/");
  }
  {
    strobe::Path<> path("../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }
  {
    strobe::Path<> path("xyz/foo/../../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<> path(".././abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<> path("./../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<> path("./../../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }

  {
    strobe::Path<> path("./.././../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }

  {
    strobe::Path<> path("xyz/foo/../../../../abc/../xyz");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../../xyz");
  }
}

TEST(Path, NormalizationEdgeCases) {
  {
    strobe::Path<> path(".");
    path.normalize();
    ASSERT_STREQ(path.c_str(), ".");
  }
  {
    strobe::Path<> path("....");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "....");
  }
  {
    strobe::Path<> path("../..");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../..");
  }
  {
    strobe::Path<> path("foo/..");
    path.normalize();
    ASSERT_STREQ(path.c_str(), ".");
  }
  {
    strobe::Path<> path("/foo/../");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "/");
  }
  {
    strobe::Path<> path("foo//bar///baz");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "foo/bar/baz");
  }
  {
    strobe::Path<> path("////");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "/");
  }
  {
    strobe::Path<> path("C:/Abc/xyz/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "C:/Abc/xyz/");
  }
  {
    strobe::Path<> path("C:/Abc/../xyz");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "C:/xyz");
  }
  {
    strobe::Path<> path("C:Abc/../xyz");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "C:xyz");
  }
  {
    strobe::Path<> path("//server/share/dir/../file");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "//server/share/file");
  }
}
