#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/resource_id.hpp"

#include <strobe/core/containers/string.hpp>

namespace strobe::flow::detail {

struct ExternalInfo {
  ResourceId resource;
  String<allocator_ref> name;
};

} // namespace strobe::flow::detail
