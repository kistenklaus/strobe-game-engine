#pragma once

#include <cassert>
#include <vector>

#include "types/inttypes.hpp"

namespace sge {

// forward declaration
template <typename T>
struct sarray_iterator;

template <typename T>
struct sarray {
 public:
  sarray() {}
  u32 insert(const T& value) {
    // this would imply that the sarray is packed.
    if (m_empty.empty()) {
      const u32 index = m_data.size();
      m_data.push_back(value);
      m_allocated.push_back(true);
      return index;
    } else {
      u32 index = m_empty.back();
      m_empty.pop_back();
      m_data[index] = value;
      m_allocated[index] = true;
      return index;
    }
  }

  void removeAt(u32 index) {
    m_empty.push_back(index);
    m_allocated[index] = false;
  }

  u32 unpackedSize() { return m_data.size(); }

  T& operator[](const u32 index) { return m_data[index]; }

  T& at(const u32 index) {
    assertValidIndex(index);
    return (*this)[index];
  }
  T* ptrTo(const u32 index) { return &m_data[index]; }

  boolean isValid(const u32 index) {
    return index < m_data.size() && m_allocated[index];
  }
  void assertValidIndex(const u32 index) { assert(isValid(index)); }

  sarray_iterator<T> begin() { return sarray_iterator<T>(this, 0); }
  sarray_iterator<T> end() { return sarray_iterator<T>(this, m_data.size()); }

 private:
  std::vector<T> m_data;
  std::vector<u32> m_empty;
  std::vector<bool> m_allocated;
};

template <typename T>
struct sarray_iterator {
 public:
  sarray_iterator(sarray<T>* p_array, u32 index)
      : mp_array(p_array), m_index(index) {
    incrementToNextValid();
  }

  T& operator*() const { return mp_array->at(m_index); }
  T* operator->() { return mp_array->ptrTo(m_index); }

  sarray_iterator<T>& operator++() {
    m_index++;
    incrementToNextValid();
    return *this;
  }
  sarray_iterator<T> operator++(int) {
    //
    sarray_iterator<T> copy = *this;
    ++(*this);
    return copy;
  }

  friend bool operator==(const sarray_iterator<T>& a,
                         const sarray_iterator<T>& b) {
    return a.m_index == b.m_index;
  }
  friend bool operator!=(const sarray_iterator<T>& a,
                         const sarray_iterator<T>& b) {
    return a.m_index != b.m_index;
  }

  void incrementToNextValid() {
    while (m_index <= mp_array->unpackedSize() && !mp_array->isValid(m_index)) {
      m_index++;
    }
  }

 private:
  sarray<T>* mp_array;
  u32 m_index;
};

}  // namespace sge
