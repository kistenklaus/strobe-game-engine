#pragma once

#include "strobe/core/fs/file_type.hpp"
#include "strobe/core/fs/path.hpp"
#include "strobe/core/fs/stat_flags.hpp"

#include <cassert>
#include <cstddef>

namespace strobe::fs {

struct Stat;

/**
 * \brief Queries the type and size of a filesystem entry.
 * \ingroup core
 * \code{.cpp}
 * Stat stat(PathView path, StatFlags flags = StatFlags::none);
 * \endcode
 *
 * \param path Path to query.
 * \param flags Whether symbolic links should be followed.
 * \return Status information for the entry.
 * \throws std::system_error If the path cannot be queried.
 */
Stat stat(PathView path, StatFlags flags = StatFlags::none);

/**
 * \brief Filesystem entry status.
 * \ingroup core
 * \code{.cpp}
 * struct Stat {
 *   bool isFile() const noexcept;
 *   bool isDirectory() const noexcept;
 *   Type type() const noexcept;
 *   std::size_t size() const noexcept;
 * };
 * \endcode
 *
 * Stores the type and, for regular files, the size of a filesystem entry.
 */
struct Stat {
public:
  friend Stat stat(PathView, StatFlags);

  /**
   * \brief Checks whether the entry is a regular file.
   * \ingroup core
   * \code{.cpp}
   * bool isFile() const noexcept;
   * \endcode
   *
   * \return Whether the entry is a regular file.
   */
  bool isFile() const noexcept { return m_type == Type::File; }

  /**
   * \brief Checks whether the entry is a directory.
   * \ingroup core
   * \code{.cpp}
   * bool isDirectory() const noexcept;
   * \endcode
   *
   * \return Whether the entry is a directory.
   */
  bool isDirectory() const noexcept { return m_type == Type::Directory; }

  /**
   * \brief Returns the entry type.
   * \ingroup core
   * \code{.cpp}
   * Type type() const noexcept;
   * \endcode
   *
   * \return The detected filesystem entry type.
   */
  Type type() const noexcept { return m_type; }

  /**
   * \brief Returns the file size.
   * \ingroup core
   * \code{.cpp}
   * std::size_t size() const noexcept;
   * \endcode
   *
   * \return File size in bytes.
   * \attention The entry must be a regular file.
   */
  std::size_t size() const noexcept {
    assert(isFile());
    return m_size;
  }

private:
  Stat() = default;
  Type m_type;
  std::size_t m_size;
};

} // namespace strobe::fs
