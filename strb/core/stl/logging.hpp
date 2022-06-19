#pragma once
#include <iostream>

namespace strb {

template <typename T> inline void log(T x) { std::cout << x; }
template <typename T> inline void logln(T x) {
  log(x);
  log('\n');
}
template <typename T> inline void error(T x) { std::cerr << x; }
template <typename T> inline void errorln(T x) { std::cerr << x << '\n'; }

} // namespace strb
