#pragma once
#include <iostream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

#include "types/inttypes.hpp"

namespace sge {

template <typename>
struct is_std_vector : std::false_type {};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type {};

static const char NL = '\n';

template <typename>
struct is_pair : std::false_type {};

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type {};

template <typename T>
void print(const T& value) {
  if constexpr (is_std_vector<T>::value) {
    std::cout << '[';
    for (u32 i = 0; i < value.size(); i++) {
      if (i != 0) {
        std::cout << ", ";
      }
      print(value[i]);
    }
    std::cout << ']';
  } else if constexpr (std::is_same<T, bool>() || std::is_same<T, boolean>()) {
    std::cout << (value ? "true" : "false");
  } else if constexpr (is_pair<T>::value) {
    print('(');
    print(value.first);
    print(", ");
    print(value.second);
    print(')');
  } else {
    std::cout << value;
  }
}

void println();

template <typename T>
void println(T value) {
  print(value);
  println();
}

template <typename T>
void printMatrix(std::vector<std::vector<T>>& mat) {
  for (u32 i = 0; i < mat.size(); i++) {
    print(i);
    print(": ");
    println(mat[i]);
  }
}

}  // namespace sge
