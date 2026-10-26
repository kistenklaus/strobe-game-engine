#pragma once

#include <fmt/format.h>

namespace strobe::flow::detail {

enum class OperationKind {
  callback,
  construct,
  cutoff,
  discard,
  acquire,
  yield,
  phi,
};

} // namespace strobe::flow::detail

namespace fmt {

template <> struct formatter<strobe::flow::detail::OperationKind> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(strobe::flow::detail::OperationKind kind,
              FormatContext &context) const {
    using strobe::flow::detail::OperationKind;
    switch (kind) {
    case OperationKind::callback:
      return fmt::format_to(context.out(), "op");
    case OperationKind::construct:
      return fmt::format_to(context.out(), "construct");
    case OperationKind::cutoff:
      return fmt::format_to(context.out(), "cutoff");
    case OperationKind::discard:
      return fmt::format_to(context.out(), "discard");
    case OperationKind::acquire:
      return fmt::format_to(context.out(), "acquire");
    case OperationKind::yield:
      return fmt::format_to(context.out(), "yield");
    case OperationKind::phi:
      return fmt::format_to(context.out(), "phi");
    }
    return fmt::format_to(context.out(), "unknown");
  }
};

} // namespace fmt
