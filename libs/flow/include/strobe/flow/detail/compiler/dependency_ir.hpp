#pragma once

#include "strobe/flow/detail/compiler/cutoff_ir.hpp"

#include <strobe/core/containers/vector.hpp>

namespace strobe::flow::detail::compiler {

struct ProducerConsumerEdge {
  NodeId producer;
  NodeId consumer;
  ValueId value;
};

struct DependencyIR {
  CutoffIR lowered;
  Vector<ProducerConsumerEdge, allocator_ref> edges;
};

} // namespace strobe::flow::detail::compiler
