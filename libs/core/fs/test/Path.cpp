#include "strobe/core/fs/Path.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include <gtest/gtest.h>

// Basic allocation and deallocation
TEST(Path, Basic) {

  {
    strobe::Path<strobe::Mallocator> path("foo");
    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo");
  }
}

TEST(Path, Append) {
  {
    strobe::Path<strobe::Mallocator> path("foo");

    path.append("abc");
    path.normalize();

    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo/abc");
  }
  {
    strobe::Path<strobe::Mallocator> path("foo/");

    path.append("abc/");
    path.normalize();

    std::string v{path.c_str()};
    ASSERT_EQ(v, "foo/abc/");
  }
}

TEST(Path, Parent) {
  {
    strobe::Path<strobe::Mallocator> path("foo/abc");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "foo/");
  }

  {
    strobe::Path<strobe::Mallocator> path("");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }

  {
    strobe::Path<strobe::Mallocator> path("./");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }

  {
    strobe::Path<strobe::Mallocator> path("../");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../../");
  }
}

TEST(Path, ClimbingParent) {
  {
    strobe::Path<strobe::Mallocator> path("foo/");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "");
  }
  {
    strobe::Path<strobe::Mallocator> path("./");
    auto parent = path.parent();
    std::string w{parent.c_str()};
    ASSERT_EQ(w, "../");
  }
}

TEST(Path, BasicNormalization) {

  {
    strobe::Path<strobe::Mallocator> path("foo/./abc");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "foo/abc");
  }

  {
    strobe::Path<strobe::Mallocator> path("foo/bar/../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "foo/abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("foo/../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("xyz/foo/../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "abc/");
  }
}

TEST(Path, ClimbingNormalization) {
  {
    strobe::Path<strobe::Mallocator> path("../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../abc/");
  }
  {
    strobe::Path<strobe::Mallocator> path("../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }
  {
    strobe::Path<strobe::Mallocator> path("xyz/foo/../../../abc/");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path(".././abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("./../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("./../../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("./.././../abc/");
    path.normalize();
    // NOTE: This is a fair tradeof it's not optimal but fine.
    ASSERT_STREQ(path.c_str(), "../../abc/");
  }

  {
    strobe::Path<strobe::Mallocator> path("xyz/foo/../../../../abc/../xyz");
    path.normalize();
    ASSERT_STREQ(path.c_str(), "../../xyz");
  }
}
