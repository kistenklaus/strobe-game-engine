#pragma once
#include <cinttypes>
#include <iostream>
#include <vector>

namespace strb {

template <typename T>
using vector = std::vector<T>;

template <typename T>
std::ostream& operator<<(std::ostream& os, const strb::vector<T>& v) {
  os << "[";
  for (uint64_t i = 0; i < v.size(); i++) {
    if (i != 0) {
      os << ",";
    }
    os << v[i];
  }
  os << "]";
  return os;
}

}  // namespace strb
