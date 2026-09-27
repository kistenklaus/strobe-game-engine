#pragma once

#include "strobe/core/containers/string.hpp"
#include "strobe/core/fs/path_view.hpp"
#include "strobe/core/fs/detail/utility.hpp"
#include "strobe/core/memory/allocator_traits.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <span>
#include <string_view>
#include <utility>

namespace strobe {

/**
 * \brief Owning, dynamically sized UTF-8 filesystem path.
 * \ingroup core
 * \code{.cpp}
 * template<Allocator A = Mallocator> class Path;
 * \endcode
 *
 * \tparam A Allocator used for path storage.
 */
template <Allocator A> class Path {
public:
  friend class PathView;

  /**
   * \brief Constructs, copies, moves, assigns, and destroys paths.
   * \ingroup core
   * \code{.cpp}
   * explicit Path(const A& alloc = {}) noexcept;
   * Path(const Path&) noexcept;
   * Path(Path&&) noexcept;
   * Path& operator=(const Path&) noexcept;
   * Path& operator=(Path&&) noexcept;
   * explicit Path(PathView view, const A& alloc = {}) noexcept;
   * ~Path() noexcept;
   * \endcode
   *
   * \param view Path characters copied by the path-view constructor.
   * \param alloc Allocator used for owned path storage.
   */
  Path(const A &alloc = {}) noexcept : m_path(alloc) {}

  Path(const Path &) noexcept = default;
  Path(Path &&) noexcept = default;
  Path &operator=(const Path &) noexcept = default;
  Path &operator=(Path &&) noexcept = default;
  ~Path() noexcept = default;

  explicit Path(PathView view, const A &alloc = {}) noexcept : m_path(alloc) {
    m_path.append(view.c_str(), view.c_str() + view.size());
  }

  /**
   * \brief Returns the null-terminated path string.
   * \ingroup core
   * \code{.cpp}
   * const char* c_str() const noexcept;
   * \endcode
   *
   * \return Pointer to the owned null-terminated path.
   */
  const char *c_str() const noexcept { return m_path.c_str(); }

  /**
   * \brief Appends another owning path as a path component.
   * \ingroup core
   * \code{.cpp}
   * Path& append(const Path& other) noexcept;
   * \endcode
   *
   * \param o Path component to append.
   * \return `*this`.
   */
  Path &append(const Path &o) noexcept {
    if (m_path.empty()) {
      m_path = o.m_path;
      return *this;
    }
    if (!strobe::fs::details::has_trailing_separator(
            std::span<const char>(m_path.data(), m_path.size()))) {
      m_path.push_back('/');
    }
    assert(strobe::fs::details::has_trailing_separator(
        std::span<const char>(m_path.data(), m_path.size())));
    m_path.append(o.m_path.begin(), o.m_path.end());
    return *this;
  }

  /**
   * \brief Appends a path view as a path component.
   * \ingroup core
   * \code{.cpp}
   * Path& append(const PathView& other) noexcept;
   * \endcode
   *
   * \param o Path component to append.
   * \return `*this`.
   */
  Path &append(const PathView &o) noexcept {
    if (m_path.empty()) {
      m_path.assign(o.span().begin(), o.span().end());
      return *this;
    }
    if (!strobe::fs::details::has_trailing_separator(
            std::span<const char>(m_path.data(), m_path.size()))) {
      m_path.push_back('/');
    }
    assert(strobe::fs::details::has_trailing_separator(
        std::span<const char>(m_path.data(), m_path.size())));
    m_path.append(o.span().begin(), o.span().end());
    return *this;
  }

  /**
   * \brief Appends an owning path with a separator.
   * \ingroup core
   * \code{.cpp}
   * Path& operator/=(const Path& other) noexcept;
   * \endcode
   *
   * \param o Path component to append.
   * \return `*this`.
   */
  Path &operator/=(const Path &o) noexcept { return append(o); }

  /**
   * \brief Appends a path view with a separator.
   * \ingroup core
   * \code{.cpp}
   * Path& operator/=(const PathView& other) noexcept;
   * \endcode
   *
   * \param o Path component to append.
   * \return `*this`.
   */
  Path &operator/=(const PathView &o) noexcept { return append(o); }

  /**
   * \brief Returns the path length excluding its null terminator.
   * \ingroup core
   * \code{.cpp}
   * std::size_t size() const noexcept;
   * \endcode
   *
   * \return Number of path characters.
   */
  std::size_t size() const noexcept { return m_path.size(); }

  /**
   * \brief Returns the logical path characters.
   * \ingroup core
   * \code{.cpp}
   * std::span<const char> span() const noexcept;
   * \endcode
   *
   * \return A non-owning span over the path characters.
   */
  std::span<const char> span() const noexcept {
    return {m_path.data(), m_path.size()};
  }

  /**
   * \brief Returns the path as a string view.
   * \ingroup core
   * \code{.cpp}
   * std::string_view view() const noexcept;
   * \endcode
   *
   * \return A non-owning view excluding the null terminator.
   */
  std::string_view view() const noexcept { return {m_path.data(), size()}; }

  /**
   * \brief Returns the extension of the final path component.
   * \ingroup core
   * \code{.cpp}
   * std::string_view extension() const noexcept;
   * \endcode
   *
   * \return The extension without its leading dot, or an empty view.
   */
  std::string_view extension() const noexcept {
    return strobe::fs::details::path_extension(
        std::span<const char>(m_path.data(), m_path.size() + 1));
  }

  /**
   * \brief Returns the parent path.
   * \ingroup core
   * \code{.cpp}
   * Path parent() const noexcept;
   * \endcode
   *
   * \return A normalized path one component above this path.
   */
  Path parent() const noexcept {
    Path parent = *this;
    parent.append("../");
    parent.normalize();
    return parent;
  }

  /**
   * \brief Returns the final path component.
   * \ingroup core
   * \code{.cpp}
   * std::string_view name() const noexcept;
   * \endcode
   *
   * \return The final component without trailing separators.
   */
  std::string_view name() const noexcept {
    return strobe::fs::details::path_name(
        std::span<const char>(m_path.data(), m_path.size() + 1));
  }

  /**
   * \brief Returns a C-string pointer to the final path component.
   * \ingroup core
   * \code{.cpp}
   * const char* c_str_name() const noexcept;
   * \endcode
   *
   * \return A pointer into the owned path at the final component.
   */
  const char *c_str_name() const noexcept {
    const std::string_view value = name();
    return value.empty() ? c_str() : value.data();
  }

  /**
   * \brief Normalizes separators and dot components in place.
   * \ingroup core
   * \code{.cpp}
   * Path& normalize() noexcept;
   * \endcode
   *
   * \return `*this`.
   * \attention The operation preserves absolute roots and does not allocate
   * additional path storage.
   */
  Path &normalize() noexcept {
    if (m_path.empty()) {
      m_path.assign(".");
      return *this;
    }
    std::size_t newSize =
        strobe::fs::details::normalize_path_inplace(std::span<char>(m_path));
    m_path.resize(newSize);
    return *this;
  }

private:
  String<A> m_path;
};

template <Allocator PA>
PathView::PathView(const Path<PA> &path) noexcept
    : m_path(std::span(path.m_path.data(), path.m_path.size() + 1)) {}

/**
 * \brief Joins two owning paths with a separator.
 * \ingroup core
 * \code{.cpp}
 * template<Allocator A>
 * Path<A> operator/(Path<A> lhs, const Path<A>& rhs) noexcept;
 * \endcode
 *
 * \param lhs Base path.
 * \param rhs Path component to append.
 * \return The joined path.
 */
template <Allocator A>
Path<A> operator/(Path<A> lhs, const Path<A> &rhs) noexcept {
  lhs /= rhs;
  return lhs;
}

/**
 * \brief Joins an owning path and a path view with a separator.
 * \ingroup core
 * \code{.cpp}
 * template<Allocator A>
 * Path<A> operator/(Path<A> lhs, PathView rhs) noexcept;
 * \endcode
 *
 * \param lhs Base path.
 * \param rhs Path component to append.
 * \return The joined path.
 */
template <Allocator A> Path<A> operator/(Path<A> lhs, PathView rhs) noexcept {
  lhs /= rhs;
  return lhs;
}

} // namespace strobe
