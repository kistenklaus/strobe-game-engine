#pragma once

#include "fmt/format.h"
#include "strobe/core/fs/exists.hpp"
#include "strobe/core/fs/path.hpp"
#include "strobe/core/fs/rm_flags.hpp"
#include "strobe/core/fs/stat.hpp"
#include <stdexcept>

namespace strobe::fs {

/**
 * \brief Removes a filesystem entry.
 * \ingroup core
 * \code{.cpp}
 * void rm(PathView path, RmFlags flags = RmFlags::none);
 * void rm(PathView path, const Stat* stat, RmFlags flags = RmFlags::none);
 * \endcode
 *
 * \param path Path to remove.
 * \param stat Existing status information for the platform implementation.
 * \param flags Removal options. Directories require \p RmFlags::recursive;
 * missing paths are ignored with \p RmFlags::force.
 * \throws std::invalid_argument If the path is missing without \p force or
 * is a directory without \p recursive.
 * \throws std::system_error If the entry cannot be removed.
 */
void rm(PathView path, const Stat *stat, RmFlags flags = RmFlags::none);

inline void rm(PathView path, RmFlags flags = RmFlags::none) {
  if (!strobe::fs::exists(path)) {
    if ((flags & RmFlags::force) == RmFlags::none) {
      throw std::invalid_argument(fmt::format(
          "Cannot remove '{}': No such file or directory", path.c_str()));
    }
    return;
  }

  Stat stat = strobe::fs::stat(path);
  rm(path, &stat, flags);
}

} // namespace strobe::fs
