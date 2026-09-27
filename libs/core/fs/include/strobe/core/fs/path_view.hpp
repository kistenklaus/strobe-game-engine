#pragma once

#include "strobe/core/fs/detail/utility.hpp"
#include "strobe/core/memory/allocator_traits.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <span>
#include <string_view>

namespace strobe {

class Mallocator;
/**
 * \brief Forward declaration of an owning filesystem path.
 * \ingroup core
 * \code{.cpp}
 * template<Allocator A = Mallocator> class Path;
 * \endcode
 */
template <Allocator PA = Mallocator> class Path;

/**
 * \brief Non-owning view of a null-terminated UTF-8 path.
 * \ingroup core
 * \code{.cpp}
 * class PathView;
 * \endcode
 *
 * \attention The referenced characters must remain alive while the view is
 * used. The view does not own or copy the path.
 */
class PathView {
public:
  /**
   * \brief Constructs, copies, moves, assigns, and destroys path views.
   * \ingroup core
   * \code{.cpp}
   * PathView() noexcept;
   * PathView(std::span<const char> path) noexcept;
   * PathView(const char* cpath) noexcept;
   * template<Allocator A> PathView(const Path<A>& path) noexcept;
   * PathView(const PathView&) noexcept;
   * PathView(PathView&&) noexcept;
   * PathView& operator=(const PathView&) noexcept;
   * PathView& operator=(PathView&&) noexcept;
   * ~PathView() noexcept;
   * \endcode
   *
   * \param path Null-terminated UTF-8 path characters when constructed from a
   * span.
   * \param cpath Null-terminated UTF-8 C string.
   * \attention Span and C-string inputs must remain valid while viewed. Span
   * inputs must end in `\0` and contain no earlier null character.
   */
  PathView() noexcept : m_path(empty_path) {}

  PathView(std::span<const char> path) noexcept : m_path(path) {
    assert(!path.empty() && path.back() == '\0');
    assert(path.empty() ||
           std::find(path.begin(), path.end() - 1, '\0') == path.end() - 1);
  }
  PathView(const char *cpath) noexcept
      : m_path(cpath ? std::span<const char>(cpath, std::strlen(cpath) + 1)
                     : std::span<const char>{}) {
    assert(cpath != nullptr);
  }

  template <Allocator PA> PathView(const Path<PA> &path) noexcept;

  /**
   * \brief Returns the null-terminated path characters.
   * \ingroup core
   * \code{.cpp}
   * const char* c_str() const noexcept;
   * \endcode
   *
   * \return Pointer to the referenced null-terminated path.
   */
  const char *c_str() const noexcept { return m_path.data(); }

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
    return fs::details::path_extension(m_path);
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
    return fs::details::path_name(m_path);
  }

  /**
   * \brief Returns a C-string pointer to the final path component.
   * \ingroup core
   * \code{.cpp}
   * const char* c_str_name() const noexcept;
   * \endcode
   *
   * \return A pointer into the viewed path at the final component.
   */
  const char *c_str_name() const noexcept {
    const std::string_view value = name();
    return value.empty() ? c_str() : value.data();
  }

  /**
   * \brief Returns the path length excluding its null terminator.
   * \ingroup core
   * \code{.cpp}
   * std::size_t size() const noexcept;
   * \endcode
   *
   * \return The number of path characters.
   */
  std::size_t size() const noexcept {
    // without null terminator
    return m_path.size() - 1;
  }

  /**
   * \brief Returns the logical path characters without the terminator.
   * \ingroup core
   * \code{.cpp}
   * std::span<const char> span() const noexcept;
   * \endcode
   *
   * \return A non-owning span over the path characters.
   */
  std::span<const char> span() const noexcept { return m_path.first(size()); }

  /**
   * \brief Returns the logical path as a string view.
   * \ingroup core
   * \code{.cpp}
   * std::string_view view() const noexcept;
   * \endcode
   *
   * \return A non-owning view excluding the null terminator.
   */
  std::string_view view() const noexcept { return {m_path.data(), size()}; }

private:
  static constexpr char empty_path[] = "";
  std::span<const char> m_path;
};

} // namespace strobe

#include "strobe/core/fs/path.hpp"
