#pragma once

#include "strobe/core/containers/string.hpp"
#include "strobe/core/fs/utility.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>
#include <span>
#include <string_view>

namespace strobe {

template <Allocator PA> class Path;

class PathView {
public:
  PathView() : m_path{} {}

  // Takes a null terminated string.
  PathView(std::span<const char> path) : m_path(path) {}
  PathView(const char* cpath) : m_path(cpath, std::strlen(cpath) + 1) {}

  template <Allocator PA> PathView(const Path<PA> &path);

  const char *c_str() const { return m_path.data(); }

  std::string_view extension() const {
    assert(isFile());
    auto rit = std::find(m_path.rbegin(), m_path.rend(), '.');
    if (rit == m_path.rend()) {
      return "";
    }
    // NOTE: it::base skips one.
    return std::string_view(rit.base(), m_path.end());
  }

  std::string_view name() const {
    if (isFile()) {
      auto rit = std::find(m_path.rbegin(), m_path.rend(), '/');
      // NOTE: it::base skips one. (so we do not include /)
      return std::string_view(rit.base(), m_path.end());
    } else {
      auto rit = std::find(m_path.rbegin() + 1, m_path.rend(), '/');
      if (rit == m_path.rend()) {
        return std::string_view(m_path.begin(), m_path.end() - 1);
      } else {
        return std::string_view(rit.base(), m_path.end() - 1);
      }
    }
  }

  bool isDirectory() const { return m_path.back() == '/'; }

  bool isFile() const { return !isDirectory(); }

  std::size_t size() const {
    // without null terminator
    return m_path.size() - 1;
  }

  std::span<const char> span() const { return m_path; }

private:
  std::span<const char> m_path;
};

template <Allocator A> class Path {
public:
  friend class PathView;
  Path([[maybe_unused]] const A &alloc = {}) : m_path(/*TODO*/) {}

  // explicit Path(const String<A> &path) : m_path(path) {}
  
  // explicit Path(String<A> &&path) : m_path(std::move(path)) {}

  explicit Path(PathView view, [[maybe_unused]] const A &alloc = {})
      : m_path(view.c_str(), view.c_str() + view.size() /*TODO all alloc*/) {}

  const char *c_str() const { return m_path.c_str(); }

  Path &append(const Path &o) {
    if (m_path.empty()) {
      m_path = o.m_path;
      return *this;
    }
    if (!isDirectory()) {
      m_path.push_back('/');
    }
    assert(isDirectory());
    m_path.append(o.m_path.begin(), o.m_path.end());
    return *this;
  }

  Path &append(const PathView &o) {
    if (m_path.empty()) {
      m_path.assign(o.span().begin(), o.span().end());
      return *this;
    }
    if (!isDirectory()) {
      m_path.push_back('/');
    }
    assert(isDirectory());
    m_path.append(o.span().begin(), o.span().end());
    return *this;
  }

  std::string_view extension() const {
    assert(isFile());
    auto rit = std::find(m_path.rbegin(), m_path.rend(), '.');
    if (rit == m_path.rend()) {
      return "";
    }
    // NOTE: it::base skips one.
    return std::string_view(rit.base(), m_path.end());
  }

  Path parent() const {
    Path parent = *this;
    parent.append("../");
    parent.normalize();
    return parent;
  }

  std::string_view name() const {
    if (isFile()) {
      auto rit = std::find(m_path.rbegin(), m_path.rend(), '/');
      // NOTE: it::base skips one. (so we do not include /)
      return std::string_view(rit.base(), m_path.end());
    } else {
      auto rit = std::find(m_path.rbegin() + 1, m_path.rend(), '/');
      if (rit == m_path.rend()) {
        return std::string_view(m_path.begin(), m_path.end() - 1);
      } else {
        return std::string_view(rit.base(), m_path.end() - 1);
      }
    }
  }

  /// Should generally not allocate memory.
  Path &normalize() {
    std::size_t newSize =
        strobe::fs::details::normalize_path_inplace(std::span<char>(m_path));
    m_path.resize(newSize);
    return *this;
  }

  bool isDirectory() const { return m_path.back() == '/'; }

  bool isFile() const { return !isDirectory(); }

private:
  String<A> m_path;
};

template <Allocator PA>
PathView::PathView(const Path<PA> &path)
    : m_path(std::span(path.m_path.data(), path.m_path.size() + 1)) {}

} // namespace strobe
