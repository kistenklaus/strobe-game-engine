#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/projection.hpp"
#include "strobe/flow/resource_id.hpp"

#include <strobe/core/containers/string.hpp>

#include <functional>
#include <optional>

namespace strobe::flow::detail {

struct ProjectionInfo {
  ResourceId resource;
  ResourceId owner;
  ProjectionKind kind;
  String<allocator_ref> name;
  std::move_only_function<void *(void *) const> resolve;
  std::optional<ExternalOp> external;
};

} // namespace strobe::flow::detail
