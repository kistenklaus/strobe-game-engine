#pragma once

#include "strobe/flow/detail/compiler/dependency_ir.hpp"

#include <strobe/core/containers/vector.hpp>

namespace strobe::flow::detail::compiler {

struct ResourceOrderConstraint {
  NodeId before;
  NodeId after;
  ResourceId resource;
};

struct OrderedIR {
  DependencyIR dependencies;
  Vector<ResourceOrderConstraint, allocator_ref> resource_order;
};

} // namespace strobe::flow::detail::compiler
