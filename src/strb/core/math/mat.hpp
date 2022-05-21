#pragma once
#include "strb/core/math/vec.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>

namespace strb::math {

template <size_t N, size_t M, typename T = double> class mat {
private:
  T *const heap_memory;
  size_t *const pointer_count;
  // utility methods
  constexpr const size_t column_size() const { return N; }
  constexpr const size_t row_size() const { return M; }
  constexpr const size_t entries() const { return M * N; }

public:
  mat() : heap_memory(new T[entries()]), pointer_count(new size_t) {
    for (size_t i = 0; i < entries(); i++) {
      heap_memory[i] = 0;
    }
    (*pointer_count) = 1;
  }

  mat(std::array<T, N * M> mat)
      : heap_memory(new T[entries()]), pointer_count(new size_t) {
    for (size_t column = 0; column < columns(); column++) {
      for (size_t row = 0; row < rows(); row++) {
        heap_memory[column * column_size() + row] =
            mat[row * row_size() + column];
      }
    }
    (*pointer_count) = 1;
  }

  mat(const mat<N, M, T> &mat)
      : heap_memory(mat.heap_memory), pointer_count(mat.pointer_count) {
    (*pointer_count)++;
  }

  mat(const mat<N, M, T> &&mat)
      : heap_memory(mat.heap_memory), pointer_count(mat.pointer_count) {}

  ~mat() {
    (*pointer_count)--;
    if ((*pointer_count) == 0) {
      delete heap_memory;
      delete pointer_count;
    }
  }

  mat<N, M, T> &operator=(const mat<N, M, T> &m) { return *this; }

  mat<N, M, T> &operator=(const mat<N, M, T> &&m) { return *this; }

  constexpr size_t rows() const { return N; }

  constexpr size_t columns() const { return M; }

  inline vec<N, T> operator[](const size_t column_index) const {
    assert(column_index < M);
    return vec<N, T>(heap_memory + column_size() * column_index, pointer_count);
  }

  inline T &at(const size_t row, const size_t column) const {
    return heap_memory[column * column_size() + row];
  }

  inline mat<N, M, T> operator+(const mat<N, M, T> &mat) {
    ::mat<N, M, T> res;
    for (size_t i = 0; i < entries(); i++) {
      res[i] = heap_memory[i] + mat.heap_memory[i];
    }
  }

  inline mat<N, M, T> operator+=(const mat<N, M, T> &mat) {
    for (size_t i = 0; i < entries(); i++) {
      heap_memory[i] += mat.heap_memory[i];
    }
  }

  inline mat<N, M, T> operator-(const mat<N, M, T> &mat) {
    ::mat<N, M, T> res;
    for (size_t i = 0; i < entries(); i++) {
      res[i] = heap_memory[i] + mat.heap_memory[i];
    }
  }

  inline mat<N, M, T> operator-=(const mat<N, M, T> &mat) {
    for (size_t i = 0; i < entries(); i++) {
      heap_memory[i] -= mat.heap_memory[i];
    }
  }

  inline void transpose() {
    static_assert(N == M, "can't transpose non quadratic maticies");
    for (size_t row = 1; row < rows(); row++) {
      for (size_t column = 0; column < row; column++) {
        // swap [n][m] and [m][n]!
        T tmp = heap_memory[column * column_size() + row];
        heap_memory[column * column_size() + row] =
            heap_memory[row * row_size() + column];
        heap_memory[row * row_size() + column] = tmp;
      }
    }
  }

  /// native implementation but probably good enoth for know
  template <size_t K> inline mat<N, K, T> operator*(const mat<M, K, T> &mat) {
    ::mat<N, K, T> res;
    for (size_t row = 0; row < res.rows(); row++) {
      for (size_t column = 0; column < res.columns(); column++) {
        for (size_t k = 0; k < M; k++) {
          res.at(row, column) += (*this).at(row, k) * mat.at(k, column);
        }
      }
    }
    return res;
  }

  inline bool operator==(const mat<N, M, T> &mat) const {
    for (size_t i = 0; i < entries(); i++) {
      if (mat.heap_memory[i] != this->heap_memory[i])
        return false;
    }
    return true;
  }

  inline vec<N, T> operator*(const vec<M, T> &vec) const {
    ::vec<N, T> res;
    for (size_t n = 0; n < columns(); n++) {
      res += (*this)[n] * vec[n];
    }
    return res;
  }

  inline mat<N, M, T> clone() const {
    mat<N, M, T> clone;
    for (size_t i = 0; i < entries(); i++) {
      clone.heap_memory[i] = this->heap_memory[i];
    }
    return clone;
  }

  friend std::ostream &operator<<(std::ostream &os, const mat<N, M, T> &mat) {

    for (size_t row = 0; row < mat.rows(); row++) {
      std::cout << "(";
      for (size_t column = 0; column < mat.columns(); column++) {
        if (column)
          std::cout << ", ";
        std::cout << mat.heap_memory[column * mat.column_size() + row];
      }
      std::cout << ")";
      if (row != mat.rows() - 1)
        std::cout << std::endl;
    }
    return os;
  }
};

} // namespace strb::math
