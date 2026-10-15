#pragma once

#include "strobe/flow/resource_id.hpp"

#include <cstddef>
#include <optional>

namespace strobe::flow::detail {

struct PhiInfo {
  ResourceId output;
  ResourceId initial;
  std::optional<ResourceId> feedback;
  std::optional<ResourceId> condition;
  std::size_t position;
  std::optional<std::size_t> feedback_position;
};

} // namespace strobe::flow::detail
