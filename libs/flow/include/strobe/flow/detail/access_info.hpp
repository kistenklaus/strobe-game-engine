#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/resource_id.hpp"

namespace strobe::flow::detail {

struct AccessInfo {
  ResourceId resource;
  AccessMode mode;
};

} // namespace strobe::flow::detail
