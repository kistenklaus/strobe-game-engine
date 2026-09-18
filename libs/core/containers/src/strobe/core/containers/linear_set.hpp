#pragma once

#include <algorithm>
#include <concepts>

#include "strobe/core/containers/vector.hpp"
namespace strobe {

template <std::equality_comparable T, Allocator A = strobe::Mallocator>
class LinearSet {
public:
  using Container = Vector<T, A>;
  using iterator = Container::iterator;
  using const_iterator = Container::iterator;
  using size_type = Container::size_type;

  LinearSet(const A &alloc = {}) : m_values(alloc) {}

  const_iterator find(const T &v) const {
    return std::ranges::find(m_values, v);
  }

  template <std::equality_comparable_with<T> K> iterator find(const K &v) {
    if constexpr (std::same_as<K, T>) {
      return std::ranges::find(m_values, v);
    } else {
      return std::ranges::find_if(m_values, [&](const T &o) { return v == o; });
    }
  }

  template <typename U> iterator insert(U &&v) {
    auto it = find(v);
    if (it != end()) {
      return it;
    }

    m_values.push_back(std::forward<U>(v));
    return end() - 1;
  }

  void insert_unchecked(T &&v) {
    assert(!contains(v));
    m_values.push_back(std::forward<T>(v));
  }

  bool contains(const T &v) { return find(v) != m_values.end(); }
  bool contains(const T &v) const { return find(v) != m_values.end(); }

  iterator erase(const_iterator pos) {
    assert(pos != end());
    std::swap(*pos, m_values.back());
    m_values.pop_back();
    return pos;
  }

  template <std::equality_comparable_with<T> K> bool erase(const K &v) {
    auto it = find(v);
    if (it == end()) {
      return false;
    }
    erase(it);
    return true;
  }

  size_type size() const { return m_values.size(); }

  bool empty() const { return m_values.empty(); }

  void clear() { m_values.clear(); }

  auto begin() { return m_values.begin(); }
  auto begin() const { return m_values.begin(); }
  auto cbegin() const { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto end() const { return m_values.end(); }
  auto cend() const { return m_values.end(); }

  auto rbegin() { return m_values.rbegin(); }
  auto rbegin() const { return m_values.rbegin(); }
  auto rcbegin() const { return m_values.rcbegin(); }
  auto rend() { return m_values.rend(); }
  auto rend() const { return m_values.rend(); }
  auto rcend() const { return m_values.rcend(); }

private:
  Container m_values;
};

} // namespace strobe
