#pragma once

#include "strobe/core/fs/copy_flags.hpp"
#include "strobe/core/fs/path.hpp"

namespace strobe::fs {

/**
 * \brief Copies a file or directory.
 * \ingroup core
 * \code{.cpp}
 * void cp(PathView src, PathView dst, CpFlags flags = CpFlags::none);
 * \endcode
 *
 * \param src Source path.
 * \param dst Destination path.
 * \param flags Copy options. Directories require \p CpFlags::recursive.
 * \attention \p dst is overwritten for files when the operation permits it.
 * \throws std::system_error If the source cannot be read or the destination
 * cannot be written.
 */
void cp(PathView src, PathView dst, CpFlags flags = CpFlags::none);

} // namespace strobe::fs
