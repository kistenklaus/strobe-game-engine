#pragma once

#include "strobe/flow/resource_id.hpp"

#include <cstddef>

namespace strobe::flow::detail {

struct IfInfo {
  ResourceId condition;
  std::size_t begin;
  std::size_t alternative;
  std::size_t end;
};

} // namespace strobe::flow::detail
