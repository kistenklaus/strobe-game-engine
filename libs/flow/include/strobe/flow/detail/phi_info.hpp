#pragma once

#include "strobe/flow/resource_id.hpp"
#include "strobe/flow/feedback_options.hpp"

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
  std::uint32_t max_in_flight;
};

} // namespace strobe::flow::detail
