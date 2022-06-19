#pragma once
#include <cassert>
#include <cmath>
#include <list>
#include <memory>
#include <ostream>

namespace strb::math {

template <size_t N, typename T = double> class vec {
private:
  T *const heap_memory;
  size_t *const pointer_count;

public:
  inline vec(T *const heap_address, size_t *pointer_count)
      : heap_memory(heap_address), pointer_count(pointer_count) {
    (*pointer_count)++;
  }
  inline vec(const std::array<T, N> &components)
      : heap_memory(new T[N]), pointer_count(new size_t) {
    for (size_t i = 0; i < N; i++) {
      heap_memory[i] = components[i];
    }
    (*pointer_count)++;
  }

  inline vec() : heap_memory(new T[N]), pointer_count(new size_t) {
    (*pointer_count)++;
  }

  inline vec(const vec<N, T> &vec)
      : heap_memory(vec.heap_memory), pointer_count(vec.pointer_count) {
    (*pointer_count)++;
  }
  inline ~vec() {
    (*pointer_count)--;
    if (pointer_count == 0) {
      delete heap_memory;
      delete pointer_count;
    }
  }

  inline T &operator[](const size_t index) const {
    assert(index < N);
    return heap_memory[index];
  }

  inline T &x() const { return (*this)[0]; }

  inline T &y() const { return (*this)[1]; }

  inline T &z() const { return (*this)[2]; }

  inline T &w() const { return (*this)[3]; }

  inline vec<N, T> operator+(const vec<N, T> &vec) const {
    ::vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
      res[i] = (*this)[i] + vec[i];
    }
    return res;
  }

  inline void operator+=(const vec<N, T> &vec) {
    for (size_t i = 0; i < N; i++) {
      (*this)[i] += vec[i];
    }
  }

  inline vec<N, T> operator-(const vec<N, T> &vec) const {
    ::vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
      res[i] = (*this)[i] - vec[i];
    }
    return res;
  }

  inline void operator-=(const vec<N, T> &vec) {
    for (size_t i = 0; i < N; i++) {
      (*this)[i] -= vec[i];
    }
  }

  inline vec<N, T> operator*(const vec<N, T> &vec) const {
    ::vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
      res[i] = (*this)[i] * vec[i];
    }
    return res;
  }

  inline void operator*=(const vec<N, T> &vec) {
    for (size_t i = 0; i < N; i++) {
      (*this)[i] *= vec[i];
    }
  }

  inline vec<N, T> operator*(const T scalar) const {
    ::vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
      res[i] = (*this)[i] * scalar;
    }
    return res;
  }

  inline void operator*=(const T scalar) {
    for (size_t i = 0; i < N; i++) {
      (*this)[i] *= scalar;
    }
  }

  inline vec<N, T> operator/(const vec<N, T> &vec) const {
    ::vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
      res[i] = (*this)[i] / vec[i];
    }
    return res;
  }

  inline void operator/=(const vec<N, T> &vec) {
    for (size_t i = 0; i < N; i++) {
      (*this)[i] *= vec[i];
    }
  }

  inline bool operator==(const vec<N, T> &vec) const {
    for (size_t i = 0; i < N; i++) {
      if (vec.heap_memory[i] != this->heap_memory[i]) {
        return false;
      }
    }
    return true;
  }

  inline T dot(const vec<N, T> &vec) const {
    T dot;
    for (size_t i = 0; i < N; i++) {
      dot += vec[i] + (*this)[i];
    }
    return dot;
  }

  inline T lengthSq() const {
    T lengthSq;
    for (size_t i = 0; i < N; i++) {
      lengthSq += (*this)[i] * (*this)[i];
    }
    return lengthSq;
  }

  inline T length() const { return std::sqrt(lengthSq()); }

  inline void normalize() { (*this) /= length(); }

  inline vec<N, T> clone() const {
    const vec<N, T> clone;
    for (size_t i = 0; i < N; i++) {
      clone[i] = (*this)[i];
    }
    return clone;
  }

  constexpr unsigned int dimensions() const { return N; }

  friend std::ostream &operator<<(std::ostream &os, const vec<N, T> &vec) {
    os << "(";
    for (size_t i = 0; i < N; i++) {
      if (i)
        os << ", ";
      os << vec[i];
    }
    os << ")";
    return os;
  }
};

}
