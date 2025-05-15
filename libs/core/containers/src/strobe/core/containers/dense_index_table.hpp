#pragma once

#include "strobe/core/containers/index_pool.hpp"

namespace strobe {

template <typename K, typename V, Allocator A>
class DenseIndexTable {
 public:
  DenseIndexTable() {}

  K add(const V& v) {
    std::size_t index = m_values.size();
    K key = m_indexPool.insert(index);
    if (key == m_values.size()) {
      m_values.push_back(v);
      m_keys.push_back(key);
    } else {
      m_values[index] = v;
      m_keys[index] = key;
    }
    return key;
  }

  void remove(const K& key) {
    assert(!m_values.empty());
    assert(m_values.size() == m_keys.size());

    std::size_t index = m_indexPool[key];
    std::size_t lastIndex = m_values.size() - 1;

    if (index != lastIndex) {
      m_values[index] = std::move(m_values.back());
      m_keys[index] = std::move(m_keys.back());

      m_indexPool[m_keys[index]] = index;
    }

    m_values.pop_back();
    m_keys.pop_back();
    m_indexPool.erase(key);
  }

 private:
  Vector<V, A> m_values;
  Vector<K> m_keys;
  IndexPool<K, std::size_t, A> m_indexPool;
};

}  // namespace strobe
