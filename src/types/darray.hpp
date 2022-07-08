#pragma once
#include <cstring>
#include <memory>

#include "types/inttypes.hpp"

namespace sge {

template <typename T>
struct darray {
 private:
  static const u32 DEFAULT_CAPACITY;

 public:
  darray()
      : m_array(reinterpret_cast<T*>(new char[DEFAULT_CAPACITY * sizeof(T)])),
        m_size(0),
        m_cap(DEFAULT_CAPACITY) {}
  explicit darray(u32 inital)
      : m_array(reinterpret_cast<T*>(new char[inital * sizeof(T)])),
        m_size(inital),
        m_cap(inital) {
    for (u32 i = 0; i < m_size; i++) {
      m_array[i] = T();
    }
  }
  explicit darray(u32 inital, T& value)
      : m_array(reinterpret_cast<T*>(new char[inital * sizeof(T)])),
        m_size(inital),
        m_cap(inital) {
    for (u32 i = 0; i < m_size; i++) {
      m_array[i] = value;
    }
  }
  ~darray() { delete m_array; }

  darray(darray<T>& other) = delete;  // TODO implement copy and move operators
  darray(darray<T>&& other) = delete;

  T& operator[](const u32 index) { return m_array[index]; }

  void push_back(const T& value) {
    if (m_size == m_cap) grow();
    m_array[m_size] = value;
  }

  T& back() {
    if (m_size == 0)
      throw std::runtime_error("can't get back() of empty darray");
    return m_array[m_size - 1];
  }

  void pop_back() {
    if (m_size == 0) throw std::runtime_error("can't pop empty darray");
    m_size--;
  }

  T& front() {
    if (m_size == 0)
      throw std::runtime_error("can't get front() of empty darray");
    return m_array[0];
  }
  struct iterator {
    explicit iterator(darray<T>* p_array, u32 index)
        : mp_parent(p_array), m_index(index) {}
    T& operator*() const { return mp_parent[index]; }
    T* operator->() { return &mp_parent[index]; }
    iterator& operator++() {
      m_index++;
      return *this;
    }
    iterator operator++(int) {
      iterator copy = *this;
      ++(*this);
      return copy;
    }
    bool operator==(const iterator& other) const {
      return m_index == other.m_index;
    }
    bool operator!=(const iterator& other) const {
      return m_index != other.m_index;
    }

   private:
    darray<T>* mp_parent;
    u32 m_index;
  };
  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, m_size); }

 private:
  void grow() {
    const u32 newCap = (m_cap * 2 / 3) + 1;
    T* newArray = new T[newCap];
    std::memcpy(newArray, m_array, m_size * sizeof(T));
    m_array = newArray;
    m_cap = newCap;
  }

 private:
  T* m_array;
  u32 m_size;
  u32 m_cap;
};

}  // namespace sge
