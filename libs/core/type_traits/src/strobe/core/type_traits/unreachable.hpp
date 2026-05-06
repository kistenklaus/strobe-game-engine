#pragma once

#include <cstdlib>
#include <utility>

namespace strobe {

[[noreturn]] inline void unreachable() noexcept {
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
  std::unreachable();
#elif defined(__GNUC__) || defined(__clang__)
  __builtin_unreachable();
#elif defined(_MSC_VER)
  __assume(false);
#else
  std::abort();
#endif
}

} // namespace strobe
