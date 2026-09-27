#pragma once

#include "strobe/core/fs/path.hpp"
namespace strobe::fs {

/**
 * \brief Checks whether a filesystem entry exists.
 * \ingroup core
 * \code{.cpp}
 * bool exists(PathView path);
 * \endcode
 *
 * \param path Path to query.
 * \return Whether the path names an existing filesystem entry.
 */
bool exists(PathView path);

} // namespace strobe::fs
