#pragma once

#include <fmt/format.h>

namespace strobe::flow {

/**
 * \brief Identifies how an operation accesses a resource.
 * \code{.cpp}
 * enum class AccessMode;
 * \endcode
 */
enum class AccessMode {
  read,
  write,
  read_write,
  consume,
};

} // namespace strobe::flow

namespace fmt {

/**
 * \brief Formats a Flow resource access mode.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::AccessMode>;
 * \endcode
 */
template <> struct formatter<strobe::flow::AccessMode> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(strobe::flow::AccessMode mode, FormatContext &context) const {
    using strobe::flow::AccessMode;
    switch (mode) {
    case AccessMode::read:
      return fmt::format_to(context.out(), "r");
    case AccessMode::write:
      return fmt::format_to(context.out(), "w");
    case AccessMode::read_write:
      return fmt::format_to(context.out(), "rw");
    case AccessMode::consume:
      return fmt::format_to(context.out(), "c");
    }
    return fmt::format_to(context.out(), "unknown");
  }
};

} // namespace fmt
