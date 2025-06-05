#pragma once

#include <type_traits>

namespace strobe {

enum class FileSeekFlags : unsigned int {
  Set = 0,  // SEEK_SET
  Cur = 1,  // SEEK_CUR
  End = 2   // SEEK_END
};


} // namespace strobe
