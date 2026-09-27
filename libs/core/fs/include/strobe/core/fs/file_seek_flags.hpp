#pragma once

namespace strobe::fs {

/**
 * \brief Origin used by file seek operations.
 * \ingroup core
 * \snippet{.cpp} file_seek_flags.hpp FileSeekFlags
 */
// [FileSeekFlags]
enum class FileSeekFlags : unsigned int {
  Set = 0,
  Cur = 1,
  End = 2,
};
// [FileSeekFlags]

} // namespace strobe::fs
