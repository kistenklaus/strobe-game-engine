#pragma once

#include <cassert>
#include <limits>
#include <type_traits>

#include "strobe/core/containers/vector.hpp"
namespace strobe {

template <std::unsigned_integral K, typename V, Allocator A>
class IndexPool {
 private:
  // End of list sential.
  static constexpr K EOL = std::numeric_limits<K>::max();
  union Entry {
    K next;
    V value;

    Entry() : value(V{}) {}
    Entry(V v) : value(std::move(v)) {}
    Entry(const Entry& o) : value(o.value) {}
    Entry& operator=(const Entry& o) {
      value = o.value;
      return *this;
    }
    Entry(Entry&& o) : value(std::move(o.value)) {}
    Entry& operator=(Entry&& o) {
      if (this == &o) {
        return *this;
      }
      value = std::move(o.value);
      return *this;
    }
    ~Entry() {}
  };

 public:
  IndexPool(const A& alloc = {}) : m_storage(alloc), m_freelistHead(EOL) {}

  ~IndexPool() {
    if (!std::is_trivially_destructible_v<V>) {
      clear();
    }
  }

  K insert()
    requires(std::is_default_constructible_v<V>)
  {
    K key;
    if (m_freelistHead == EOL) {
      key = m_storage.size();
      m_storage.push_back(Entry{});
    } else {
      key = m_freelistHead;
      m_freelistHead = m_storage[key].next;
      new (&m_storage[key].value) V();
    }
    return key;
  }

  K insert(V v) {
    K key;
    if (m_freelistHead == EOL) {
      key = m_storage.size();
      m_storage.push_back(Entry(std::move(v)));
    } else {
      key = m_freelistHead;
      m_freelistHead = m_storage[key].next;
      new (&m_storage[key].value) Entry(std::move(v));
    }
    return key;
  }

  V& operator[](const K& k) {
    assert(contains(k));
    return m_storage[k].value;
  }

  const V& operator[](const K& k) const {
    assert(contains(k));
    return m_storage[k].value;
  }

  void erase(const K& k) {
    assert(contains(k));
    m_storage[k].value.~V();
    m_storage[k].next = m_freelistHead;
    m_freelistHead = k;
  }

  void clear() {
    for (std::size_t i = 0; i < m_storage.size(); ++i) {
      K k = i;
      if (contains(k)) {
        m_storage[i].value.~V();
      }
    }
    m_storage.clear();
    m_freelistHead = EOL;
  }

 private:
  bool contains(const K& k) {
    if (k >= m_storage.size()) {
      return false;
    }
    K current = m_freelistHead;
    bool active = true;
    while (current != EOL) {
      if (current == k) {
        active = false;
        break;
      }
      current = m_storage[current].next;
    }
    return active;
  }

  Vector<Entry, A> m_storage;
  K m_freelistHead;
};

}  // namespace strobe
