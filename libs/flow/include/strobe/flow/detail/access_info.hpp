#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/access_purpose.hpp"
#include "strobe/flow/resource_id.hpp"

#include <fmt/format.h>

namespace strobe::flow::detail {

struct AccessInfo {
  ResourceId resource;
  AccessMode mode;
  AccessPurpose purpose = AccessPurpose::argument;
};

} // namespace strobe::flow::detail

namespace fmt {

template <> struct formatter<strobe::flow::detail::AccessInfo> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(const strobe::flow::detail::AccessInfo &access,
              FormatContext &context) const {
    if (access.purpose == strobe::flow::AccessPurpose::serialization) {
      return fmt::format_to(context.out(), "s:%{}", access.resource);
    }
    return fmt::format_to(context.out(), "{}:%{}", access.mode,
                          access.resource);
  }
};

} // namespace fmt
