#include "strobe/core/fs/File.hpp"
#include "strobe/core/fs/FileAccessFlags.hpp"
#include "strobe/core/fs/rm.hpp"
#include <print>
#include <strobe/core/fs/Path.hpp>
#include <strobe/core/fs/mkdir.hpp>
#include <strobe/core/memory/Mallocator.hpp>

int main() {
  strobe::Path<strobe::Mallocator> path("foo");

  path.append("abc");
  path.normalize();

  std::string v{path.c_str()};


  // strobe::Path<strobe::Mallocator> path("./fs-testing/abc/xyz");
  //
  // strobe::fs::mkdir(path, strobe::fs::MkdirFlagBits::Parents);
  //
  // strobe::Path<strobe::Mallocator> filepath("./fs-testing/abc/foo.txt");
  // strobe::File file{filepath, strobe::FileAccessBits::Create |
  //                                 strobe::FileAccessBits::Write |
  //                                 strobe::FileAccessBits::Exclusive |
  //                                 strobe::FileAccessBits::Trunc};
  //
  // strobe::File file3{filepath, strobe::FileAccessBits::Create |
  //                                  strobe::FileAccessBits::Write |
  //                                  strobe::FileAccessBits::Trunc};
  //
  // std::println("path-view: {}", path.c_str());
  //
  // // strobe::File file2{filepath, strobe::FileAccessBits::Read};
  // //
  // // strobe::Path<strobe::Mallocator> rmPath("./fs-testing/abc/xyz");
  // // strobe::fs::rm(rmPath, strobe::fs::RmFlagBits::Recursive);
  // //
  // strobe::Path<strobe::Mallocator> testingRoot("./fs-testing/");
  // strobe::fs::rm(testingRoot, strobe::fs::RmFlagBits::Recursive);
}
