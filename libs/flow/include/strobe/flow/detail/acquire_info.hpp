#pragma once

#include "strobe/flow/allocator.hpp"

#include <strobe/core/containers/string.hpp>

#include <cstddef>

namespace strobe::flow::detail {

struct AcquireInfo {
  std::size_t operation;
  String<allocator_ref> name;
};

} // namespace strobe::flow::detail
