#pragma once

#include "fixed_string.hpp"
#include <string_view>

namespace strobe {

template <typename T> consteval std::string_view type_name_view() noexcept {
#if defined(__clang__)
  constexpr std::string_view signature = __PRETTY_FUNCTION__;
  constexpr std::string_view prefix = "T = ";
  constexpr std::string_view suffix = "]";

#elif defined(__GNUC__)
  constexpr std::string_view signature = __PRETTY_FUNCTION__;
  constexpr std::string_view prefix = "T = ";
  constexpr std::string_view suffix = ";";

#elif defined(_MSC_VER)
  constexpr std::string_view signature = __FUNCSIG__;
  // MSVC-specific parsing.

#else
  return {};
#endif

  const auto begin = signature.find(prefix) + prefix.size();

  const auto end = signature.find(suffix, begin);

  return signature.substr(begin, end - begin);
}

template <typename T> consteval auto type_name() noexcept {
  constexpr auto view = type_name_view<T>();

  if constexpr (view.empty()) {
    return fixed_string{"<unnamed>"};
  } else {
    return fixed_string_from_view<view.size()>(view);
  }
}

}
