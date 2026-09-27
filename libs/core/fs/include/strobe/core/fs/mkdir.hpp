#pragma once

#include "strobe/core/fs/mkdir_flags.hpp"
#include "strobe/core/fs/path.hpp"

namespace strobe::fs {

/**
 * \brief Creates a directory.
 * \ingroup core
 * \code{.cpp}
 * void mkdir(PathView path, MkdirFlags flags = MkdirFlags::none);
 * \endcode
 *
 * \param path Directory path.
 * \param flags Creation options. \p MkdirFlags::parents creates missing
 * parent directories.
 * \throws std::system_error If the directory cannot be created.
 */
void mkdir(PathView path, MkdirFlags flags = MkdirFlags::none);

/**
 * \brief Creates a directory from an owning path.
 * \ingroup core
 * \code{.cpp}
 * template<Allocator A>
 * void mkdir(const Path<A>& path, MkdirFlags flags = MkdirFlags::none);
 * \endcode
 *
 * \param path Owning directory path.
 * \param flags Creation options.
 */
template <Allocator PA>
void mkdir(const Path<PA> &path, MkdirFlags flags = MkdirFlags::none) {
  mkdir(PathView(path), flags);
}

} // namespace strobe::fs
