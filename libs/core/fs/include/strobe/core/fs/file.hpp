#pragma once

#include "strobe/core/fs/access_flags.hpp"
#include "strobe/core/fs/file_seek_flags.hpp"
#include "strobe/core/fs/path.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace strobe::fs {

class File;

/**
 * \brief Opens a file and returns its handle.
 * \ingroup core
 * \code{.cpp}
 * File open(PathView path, FileAccess access = FileAccess::read);
 * \endcode
 *
 * \param path File path.
 * \param access Access and creation options.
 * \return An opened file handle.
 * \throws std::system_error If the operating system cannot open the file.
 * \throws std::invalid_argument If the access flags are inconsistent.
 */
File open(PathView path, FileAccess access = FileAccess::read);

/**
 * \brief Movable handle for an opened file.
 * \ingroup core
 * \code{.cpp}
 * class File;
 * \endcode
 *
 * File handles are exclusive owners and cannot be copied.
 */
class File {
public:
  /**
   * \brief Constructs, moves, assigns, and destroys file handles.
   * \ingroup core
   * \code{.cpp}
   * File() noexcept;
   * ~File() noexcept;
   * File(const File&) = delete;
   * File& operator=(const File&) = delete;
   * File(File&& other) noexcept;
   * File& operator=(File&& other) noexcept;
   * \endcode
   */
  File() noexcept : m_fd(-1) {}
  ~File() noexcept;

  File(const File &) = delete;
  File &operator=(const File &) = delete;
  File(File &&other) noexcept;
  File &operator=(File &&other) noexcept;

  /**
   * \brief Closes the file handle.
   * \ingroup core
   * \code{.cpp}
   * void close();
   * \endcode
   *
   * \throws std::system_error If the operating system cannot close the file.
   */
  void close();

  /**
   * \brief Returns the current file size.
   * \ingroup core
   * \code{.cpp}
   * std::size_t size() const;
   * \endcode
   *
   * \return File size in bytes.
   * \attention The file must be open.
   * \throws std::system_error If the size cannot be queried.
   */
  std::size_t size() const;

  /**
   * \brief Reads bytes from the current file position.
   * \ingroup core
   * \code{.cpp}
   * std::size_t read(std::span<std::byte> buffer) const;
   * \endcode
   *
   * \param buffer Destination buffer.
   * \return Number of bytes read; fewer bytes may be returned at end of file.
   * \attention The file must be open.
   * \throws std::system_error If the read fails.
   */
  std::size_t read(std::span<std::byte> buffer) const;

  /**
   * \brief Writes bytes at the current file position.
   * \ingroup core
   * \code{.cpp}
   * std::size_t write(std::span<const std::byte> buffer);
   * \endcode
   *
   * \param buffer Source bytes.
   * \return Number of bytes written.
   * \attention The file must be open with write access.
   * \throws std::system_error If the write fails.
   */
  std::size_t write(std::span<const std::byte> buffer);

  /**
   * \brief Moves the current file position.
   * \ingroup core
   * \code{.cpp}
   * void seek(long offset, FileSeekFlags flags = FileSeekFlags::Set);
   * \endcode
   *
   * \param offset Offset relative to the selected origin.
   * \param flags Position origin.
   * \attention The file must be open.
   * \throws std::invalid_argument If \p flags is invalid.
   * \throws std::system_error If the position cannot be changed.
   */
  void seek(long offset, FileSeekFlags flags = FileSeekFlags::Set);

  /**
   * \brief Returns the current file position.
   * \ingroup core
   * \code{.cpp}
   * std::size_t tell() const;
   * \endcode
   *
   * \return Current position in bytes from the beginning of the file.
   * \attention The file must be open.
   * \throws std::system_error If the position cannot be queried.
   */
  std::size_t tell() const;

  /**
   * \brief Changes the file size.
   * \ingroup core
   * \code{.cpp}
   * void truncate(std::size_t new_size);
   * \endcode
   *
   * \param new_size New size in bytes.
   * \attention The file must be open with write access.
   * \throws std::system_error If the size cannot be changed.
   */
  void truncate(std::size_t new_size);

  /**
   * \brief Checks whether the handle is open.
   * \ingroup core
   * \code{.cpp}
   * bool isOpen() const noexcept;
   * \endcode
   *
   * \return Whether the file owns an open handle.
   */
  bool isOpen() const noexcept { return m_fd != -1; }

  /**
   * \brief Checks whether the handle is open.
   * \ingroup core
   * \code{.cpp}
   * operator bool() const noexcept;
   * \endcode
   *
   * \return Whether the file owns an open handle.
   */
  operator bool() const noexcept { return isOpen(); }

private:
  friend File open(PathView path, FileAccess access);

  File(PathView path, FileAccess access);

  std::intptr_t m_fd;
};

inline File open(PathView path, FileAccess access) {
  return File(path, access);
}

} // namespace strobe::fs

namespace strobe {

/**
 * \brief Opens a file through the top-level filesystem API.
 * \ingroup core
 * \code{.cpp}
 * fs::File open(PathView path, fs::FileAccess access = fs::FileAccess::read);
 * \endcode
 *
 * \param path File path.
 * \param access Access and creation options.
 * \return An opened file handle.
 */
inline fs::File open(PathView path,
                     fs::FileAccess access = fs::FileAccess::read) {
  return fs::open(path, access);
}

} // namespace strobe
