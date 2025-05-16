#pragma once

#include <algorithm>
#include <concepts>

#include "strobe/core/containers/vector.hpp"

namespace strobe {

template <std::equality_comparable K, typename V, Allocator A>
class LinearMap {
 public:
  void insert(const K& key, const V& value) {
    assert(m_keys.size() == m_values.size());
    assert(!containsKey(key));
    m_values.push_back(value);
    m_keys.push_back(key);
  }

  void insert(const K& key, V&& value) {
    assert(m_keys.size() == m_values.size());
    assert(!containsKey(key));
    m_values.push_back(std::move(value));
    m_keys.push_back(key);
  }

  bool erase(const K& key) {
    assert(m_keys.size() == m_values.size());
    auto it = std::ranges::find(m_keys, key);
    if (it == m_keys.end()) {
      return false;
    }
    std::size_t index = std::distance(m_keys.begin(), it);
    std::swap(m_keys[index], m_keys.back());
    std::swap(m_values[index], m_values.back());
    m_keys.pop_back();
    m_values.pop_back();
    return true;
  }

  bool containsKey(const K& key) const {
    auto it = std::ranges::find(m_keys, key);
    return it != m_keys.end();
  }

  auto begin() { return m_values.begin(); }
  auto begin() const { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto end() const { return m_values.end(); }

 private:
  Vector<K, A> m_keys;
  Vector<V, A> m_values;
};
}  // namespace strobe
