#pragma once

#include "strobe/core/fs/file_type.hpp"
#include "strobe/core/fs/path.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

namespace strobe::fs {

/**
 * \brief Single-pass directory enumeration.
 * \ingroup core
 * \code{.cpp}
 * class Directory;
 * \endcode
 *
 * A directory is an input range. Iterating it does not collect entries or
 * allocate memory. Entry views remain valid until the iterator is advanced.
 */
class Directory {
public:
  /**
   * \brief A non-owning directory entry result.
   * \ingroup core
   * \code{.cpp}
   * struct EntryView {
   *   PathView name;
   *   Type type;
   * };
   * \endcode
   *
   * The name contains only the entry name, not the directory path.
   * \attention \p name is valid only until the directory iterator advances.
   */
  struct EntryView {
    PathView name;
    Type type;
  };

  /**
   * \brief Single-pass input iterator over directory entries.
   * \ingroup core
   * \code{.cpp}
   * class iterator;
   * \endcode
   */
  class iterator {
  public:
    /**
     * \brief Identifies this iterator as a single-pass input iterator.
     * \ingroup core
     * \code{.cpp}
     * using iterator_category = std::input_iterator_tag;
     * \endcode
     */
    using iterator_category = std::input_iterator_tag;

    /**
     * \brief Iterator value type.
     * \ingroup core
     * \code{.cpp}
     * using value_type = EntryView;
     * \endcode
     */
    using value_type = EntryView;

    /**
     * \brief Iterator difference type.
     * \ingroup core
     * \code{.cpp}
     * using difference_type = std::ptrdiff_t;
     * \endcode
     */
    using difference_type = std::ptrdiff_t;

    /**
     * \brief Iterator pointer type.
     * \ingroup core
     * \code{.cpp}
     * using pointer = const EntryView*;
     * \endcode
     */
    using pointer = const EntryView *;

    /**
     * \brief Iterator reference type.
     * \ingroup core
     * \code{.cpp}
     * using reference = const EntryView&;
     * \endcode
     */
    using reference = const EntryView &;

    /**
     * \brief Constructs an end iterator.
     * \ingroup core
     * \code{.cpp}
     * iterator() noexcept;
     * \endcode
     */
    iterator() noexcept = default;

    /**
     * \brief Accesses the current entry.
     * \ingroup core
     * \code{.cpp}
     * const EntryView& operator*() const noexcept;
     * \endcode
     *
     * \return The current directory entry.
     * \attention The result is invalidated by incrementing the iterator.
     */
    reference operator*() const noexcept { return m_entry; }

    /**
     * \brief Accesses the current entry through a pointer.
     * \ingroup core
     * \code{.cpp}
     * const EntryView* operator->() const noexcept;
     * \endcode
     *
     * \return Pointer to the current directory entry.
     */
    pointer operator->() const noexcept { return &m_entry; }

    /**
     * \brief Advances to the next directory entry.
     * \ingroup core
     * \code{.cpp}
     * iterator& operator++();
     * iterator operator++(int);
     * \endcode
     *
     * \return The advanced iterator for prefix increment.
     * \attention The current entry view is invalidated.
     */
    iterator &operator++();
    iterator operator++(int) {
      iterator copy = *this;
      ++(*this);
      return copy;
    }

    /**
     * \brief Compares input iterators for equality.
     * \ingroup core
     * \code{.cpp}
     * friend bool operator==(const iterator& lhs,
     *                        const iterator& rhs) noexcept;
     * \endcode
     *
     * \return Whether both iterators represent the same range position.
     */
    friend bool operator==(const iterator &lhs, const iterator &rhs) noexcept {
      return lhs.m_directory == rhs.m_directory && lhs.m_end == rhs.m_end;
    }

    /**
     * \brief Compares input iterators for inequality.
     * \ingroup core
     * \code{.cpp}
     * friend bool operator!=(const iterator& lhs,
     *                        const iterator& rhs) noexcept;
     * \endcode
     *
     * \return Whether the iterators represent different positions.
     */
    friend bool operator!=(const iterator &lhs, const iterator &rhs) noexcept {
      return !(lhs == rhs);
    }

  private:
    friend class Directory;
    explicit iterator(Directory *directory) noexcept : m_directory(directory) {}

    Directory *m_directory = nullptr;
    EntryView m_entry{};
    bool m_end = true;
  };

  /**
   * \brief Constructs, moves, assigns, and destroys directory handles.
   * \ingroup core
   * \code{.cpp}
   * Directory() noexcept;
   * explicit Directory(PathView path);
   * Directory(const Directory&) = delete;
   * Directory& operator=(const Directory&) = delete;
   * Directory(Directory&& other) noexcept;
   * Directory& operator=(Directory&& other) noexcept;
   * ~Directory() noexcept;
   * \endcode
   *
   * \param path Directory path for the opening constructor.
   * \throws std::system_error If the directory cannot be opened.
   */
  Directory() noexcept = default;
  explicit Directory(PathView path);

  Directory(const Directory &) = delete;
  Directory &operator=(const Directory &) = delete;

  Directory(Directory &&other) noexcept
      : m_handle(std::exchange(other.m_handle, nullptr)),
        m_opened(std::exchange(other.m_opened, false)),
        m_platform_state(other.m_platform_state), m_name(other.m_name) {}

  Directory &operator=(Directory &&other) noexcept;
  ~Directory() noexcept;

  /**
   * \brief Returns an iterator to the first entry.
   * \ingroup core
   * \code{.cpp}
   * iterator begin();
   * \endcode
   *
   * \return Iterator to the first entry, or the end iterator for an empty
   * directory.
   */
  iterator begin();

  /**
   * \brief Returns the end iterator.
   * \ingroup core
   * \code{.cpp}
   * iterator end() const noexcept;
   * \endcode
   *
   * \return Sentinel iterator marking the end of enumeration.
   */
  iterator end() const noexcept { return {}; }

  /**
   * \brief Closes the directory handle.
   * \ingroup core
   * \code{.cpp}
   * void close();
   * \endcode
   *
   * \throws std::system_error If the operating system cannot close the handle.
   */
  void close();

  /**
   * \brief Checks whether the directory is open.
   * \ingroup core
   * \code{.cpp}
   * bool valid() const noexcept;
   * \endcode
   *
   * \return Whether the directory has an open enumeration handle.
   */
  bool valid() const noexcept { return m_opened; }

  /**
   * \brief Checks whether the directory is open.
   * \ingroup core
   * \code{.cpp}
   * explicit operator bool() const noexcept;
   * \endcode
   *
   * \return Whether the directory has an open enumeration handle.
   */
  explicit operator bool() const noexcept { return valid(); }

private:
  friend Directory ls(PathView path);

  bool advance(EntryView &entry);
  void close_platform() noexcept;

  static constexpr std::size_t platform_state_size = 32768;
  void *m_handle = nullptr;
  bool m_opened = false;
  alignas(std::max_align_t) std::array<std::byte, platform_state_size>
      m_platform_state{};
  std::array<char, 4096> m_name{};
};

/**
 * \brief Opens a directory for single-pass enumeration.
 * \ingroup core
 * \code{.cpp}
 * Directory ls(PathView path);
 * \endcode
 *
 * \param path Directory path.
 * \return An opened directory input range.
 * \throws std::system_error If the directory cannot be opened.
 */
Directory ls(PathView path);

} // namespace strobe::fs
