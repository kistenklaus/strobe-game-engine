#pragma once

#include "strobe/core/fs/mv_flags.hpp"
#include "strobe/core/fs/path.hpp"

namespace strobe::fs {

/**
 * \brief Moves a file or directory.
 * \ingroup core
 * \code{.cpp}
 * void mv(PathView src, PathView dst, MvFlags flags = MvFlags::none);
 * \endcode
 *
 * \param src Source path.
 * \param dst Destination path.
 * \param flags Move options, including replacement and timestamp preservation.
 * \throws std::system_error If the move cannot be completed.
 */
void mv(PathView src, PathView dst, MvFlags flags = MvFlags::none);

} // namespace strobe::fs
